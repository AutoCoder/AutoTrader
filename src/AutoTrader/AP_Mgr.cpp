#include "stdafx.h"
#include "AP_Mgr.h"
#include "Order.h"
#include "CommonUtils.h"
#include <memory>
#include <sstream>
#include <algorithm>

namespace AP{

	struct IsSameTrade{

		IsSameTrade(const CThostFtdcTradeField& input)
		{
			memcpy(&m_data, &input, sizeof(input));
		}

		bool operator() (const CThostFtdcTradeField& input){
			return strcmp(m_data.TradeID, input.TradeID) == 0;
		}

	private:
		CThostFtdcTradeField m_data;
	};

	struct IsSameOrder{

		IsSameOrder(const CThostFtdcOrderField& input)
		{
			memcpy(&m_data, &input, sizeof(input));
		}

		bool operator() (const CThostFtdcOrderField& input){
			return m_data.BrokerOrderSeq == input.BrokerOrderSeq;
		}

	private:
		CThostFtdcOrderField m_data;
	};

	AccountAndPositionMgr& GetManager(){
		static AccountAndPositionMgr mgr; //Scott Meyers singleton 
		return mgr;
	}

	AccountAndPositionMgr::AccountAndPositionMgr()
		: m_isReady(false)
	{
	}

	AccountAndPositionMgr::~AccountAndPositionMgr()
	{
	}


	void AccountAndPositionMgr::setAccountStatus(const CThostFtdcTradingAccountField& info){
		memcpy(&m_accountInfo, &info, sizeof(CThostFtdcTradingAccountField));
	}

	void AccountAndPositionMgr::pushTodayNewTrade(const CThostFtdcTradeField& tradeField){
		std::map<std::string, TradeMessage>::iterator iter = m_tradeMessage_dict.find(std::string(tradeField.InstrumentID));
		if (iter == m_tradeMessage_dict.end()){ // if the instrument is not existed, create it.
			TradeMessage message;
			message.InstId = tradeField.InstrumentID;
			m_tradeMessage_dict[tradeField.InstrumentID] = message;
		}

		//一个多手报单 分多次成交， 会有多次的成交通知，但每次成交编号都不一样 
		//通过TradeID，排除重复trade，确保成交记录不重复
		auto tradeIter = std::find_if(m_tradelist.begin(), m_tradelist.end(), IsSameTrade(tradeField));
		if (tradeIter != m_tradelist.end()){
			*tradeIter = tradeField;
		}
		else{
			m_tradelist.push_back(tradeField);
		}

		int close_num_account_long = 0;//平仓的多单手数，如果有的话
		int close_num_account_short = 0;//平仓的空单手数，如果有的话


		//若是开仓单，则保存到tradeList_notClosed_account_long和tradeList_notClosed_account_short

		if (tradeField.OffsetFlag == '0')//开仓
		{
			if (tradeField.Direction == '0')//多单
			{
				//多单持仓表增加记录
				m_tradeList_nonClosed_account_long.push_back(tradeField);
				//多单持仓量
				m_tradeMessage_dict[tradeField.InstrumentID].Holding_long = m_tradeMessage_dict[tradeField.InstrumentID].Holding_long + tradeField.Volume;
				//多单今日持仓
				m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long + tradeField.Volume;
			}
			else if (tradeField.Direction == '1')//空单
			{
				//空单持仓表增加记录
				m_tradeList_notClosed_account_short.push_back(tradeField);
				//空单持仓量
				m_tradeMessage_dict[tradeField.InstrumentID].Holding_short = m_tradeMessage_dict[tradeField.InstrumentID].Holding_short + tradeField.Volume;
				//空单今日持仓
				m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short + tradeField.Volume;
			}
		}
		//平仓单，删除记录
		else if (tradeField.OffsetFlag == '1' || tradeField.OffsetFlag == '3')//1是平仓，3是平今
		{
			if (tradeField.Direction == '1')//卖，表示平多,有昨仓和今仓时，按时间顺序，先平昨仓
			{
				close_num_account_long = tradeField.Volume;

				for (auto iter = m_tradeList_nonClosed_account_long.begin(); iter != m_tradeList_nonClosed_account_long.end(); iter++)
				{
					if (strcmp(tradeField.InstrumentID, iter->InstrumentID) == 0)
					{
						if (iter->Volume < close_num_account_long)//没有忽略掉tradeList_notClosed中数量为0的单子，但不影响计算结果
						{
							close_num_account_long -= iter->Volume;
							//平仓盈亏
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long + (tradeField.Price - iter->Price) * iter->Volume * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							iter->Volume = 0;
						}
						else if (iter->Volume == close_num_account_long)
						{
							iter->Volume = 0;
							//平仓盈亏
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long + (tradeField.Price - iter->Price) * close_num_account_long * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							break;
						}
						else if (iter->Volume > close_num_account_long)
						{
							iter->Volume = iter->Volume - close_num_account_long;
							//平仓盈亏
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long + (tradeField.Price - iter->Price) * close_num_account_long * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							break;
						}
					}
				}

				//多单持仓量
				m_tradeMessage_dict[tradeField.InstrumentID].Holding_long = m_tradeMessage_dict[tradeField.InstrumentID].Holding_long - tradeField.Volume;
				//今仓持仓量和昨仓量，要分上期所和非上期所


				//今仓量和昨仓量，只对上期所有效
				if (tradeField.OffsetFlag == '1')
					m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long = m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long - tradeField.Volume;//昨仓
				else if (tradeField.OffsetFlag == '3')
					m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long - tradeField.Volume;//今仓


				//假设今仓5手，昨仓1，平仓都是发'1'，假设平仓2手，导致昨仓是-1，今仓还是5手，实际应该是今仓5-1，昨仓0
				//3手昨仓，5手今仓，，'1'平仓了4手,导致昨仓是-1，今仓还是5手，实际应该是今仓5-1，昨仓0
				if (m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long < 0)
				{
					m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long + m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long;
					m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long = 0;
				}
			}
			else if (tradeField.Direction == '0')//平空
			{
				close_num_account_short = tradeField.Volume;

				for (auto iter = m_tradeList_notClosed_account_short.begin(); iter != m_tradeList_notClosed_account_short.end(); iter++)
				{
					if (strcmp(tradeField.InstrumentID, iter->InstrumentID) == 0)
					{
						if (iter->Volume < close_num_account_short)//没有忽略掉tradeList_notClosed中数量为0的单子，但不影响计算结果
						{
							close_num_account_short -= iter->Volume;
							//平仓盈亏
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short + (iter->Price - tradeField.Price) * iter->Volume * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							iter->Volume = 0;
						}

						else if (iter->Volume == close_num_account_short)
						{
							iter->Volume = 0;
							//平仓盈亏
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short + (iter->Price - tradeField.Price) * close_num_account_short * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							break;
						}

						else if (iter->Volume > close_num_account_short)
						{
							iter->Volume = iter->Volume - close_num_account_short;
							//平仓盈亏
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short + (iter->Price - tradeField.Price) * close_num_account_short * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							break;
						}

					}

				}

				//空单持仓量
				m_tradeMessage_dict[tradeField.InstrumentID].Holding_short = m_tradeMessage_dict[tradeField.InstrumentID].Holding_short - tradeField.Volume;

				//空单今日持仓
				//m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short - trade_account->Volume;

				//今仓量和昨仓量，只对上期所有效
				if (tradeField.OffsetFlag == '1')
					m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_short = m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_short - tradeField.Volume;//昨仓
				else if (tradeField.OffsetFlag == '3')
					m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short - tradeField.Volume;//今仓


				if (m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_short < 0)
				{
					m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short + m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_short;
					m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_short = 0;

				}
			}

		}
	}

	void AccountAndPositionMgr::pushTodayOrder(const CThostFtdcOrderField& orderField){
		m_orderlist.push_back(orderField);
	}

	void AccountAndPositionMgr::pushImmediateOrder(const CThostFtdcOrderField& orderField){
		auto orderIter = std::find_if(m_orderlist.begin(), m_orderlist.end(), IsSameOrder(orderField));
		if (orderIter != m_orderlist.end()){
			*orderIter = orderField;
		}
		else{
			m_orderlist.push_back(orderField);
		}
	}

	std::string AccountAndPositionMgr::todayOrderToString() const{
		return CommonUtils::ConvertOrderListToString(m_orderlist);
	}

	void AccountAndPositionMgr::pushTodayTrade(const CThostFtdcTradeField& tradeField){
		m_tradelist.push_back(tradeField);
	}

	std::string AccountAndPositionMgr::todayTradeToString() const{
		return CommonUtils::ConvertTradeListToString(m_tradelist);
	}

	void AccountAndPositionMgr::pushYesterdayUnClosedTrade(const CThostFtdcTradeField& tradeField, Direction direction){
		if (direction == THOST_FTDC_D_Buy){
			m_tradeList_nonClosed_account_long.push_back(tradeField);
		}
		else if (direction == THOST_FTDC_D_Sell){
			m_tradeList_notClosed_account_short.push_back(tradeField);
		}
	}

	std::string AccountAndPositionMgr::yesterdayUnClosedTradeToString(Direction direction){
		if (direction == THOST_FTDC_D_Buy){
			return CommonUtils::ConvertTradeListToString(m_tradeList_nonClosed_account_long);
		}
		else if (direction == THOST_FTDC_D_Sell){
			return CommonUtils::ConvertTradeListToString(m_tradeList_notClosed_account_short);
		}
		else
			return "";
	}

	void AccountAndPositionMgr::pushTradeMessage(const CThostFtdcInvestorPositionField& originalTradeStruct){
		std::map<std::string, TradeMessage>::iterator iter = m_tradeMessage_dict.find(std::string(originalTradeStruct.InstrumentID));
		if (iter == m_tradeMessage_dict.end()){ // if the instrument is not existed, create it.
			TradeMessage message;
			message.InstId = originalTradeStruct.InstrumentID;
			m_tradeMessage_dict[originalTradeStruct.InstrumentID] = message;
		}

		if (originalTradeStruct.PosiDirection == '2') //多单
		{
			//多单持仓量
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].Holding_long = originalTradeStruct.Position;
			//多单今仓
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].TodayPosition_long = originalTradeStruct.TodayPosition;
			//多单昨仓 = 多单持仓量 - 多单今仓
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].YdPosition_long = originalTradeStruct.Position - originalTradeStruct.TodayPosition;
			//多单平仓盈亏
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].closeProfit_long = originalTradeStruct.CloseProfit;
			//多单浮动盈亏(其实是持仓盈亏，按昨结算的)
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].OpenProfit_long = originalTradeStruct.PositionProfit;
		}
		else if (originalTradeStruct.PosiDirection == '3') // 空单
		{
			//空单持仓量
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].Holding_short = originalTradeStruct.Position;
			//空单今仓
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].TodayPosition_short = originalTradeStruct.TodayPosition;
			//空单昨仓 = 空单持仓量 - 空单今仓
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].YdPosition_short = originalTradeStruct.Position - m_tradeMessage_dict[originalTradeStruct.InstrumentID].TodayPosition_short;
			//空单平仓盈亏
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].closeProfit_short = originalTradeStruct.CloseProfit;
			//空单浮动盈亏
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].OpenProfit_short = originalTradeStruct.PositionProfit;
		}
	}

	double AccountAndPositionMgr::getCloseProfit(){
		double ret = 0;
		for (auto item : m_tradeMessage_dict){
			ret += (item.second.closeProfit_long + item.second.closeProfit_short);
		}
		return ret;
	}

	double AccountAndPositionMgr::getOpenProfit(){
		double ret = 0;
		for (auto item : m_tradeMessage_dict){
			ret += (item.second.OpenProfit_long + item.second.OpenProfit_short);
		}
		return ret;
	}

	void AccountAndPositionMgr::pushInstrumentStruct(const CThostFtdcInstrumentField& instru)
	{
		m_instrument_dict[instru.InstrumentID] = instru;
	}

	CThostFtdcInstrumentField AccountAndPositionMgr::getInstrumentField(const std::string& instrId) const{
		if (m_instrument_dict.count(instrId) != 0)
			return m_instrument_dict.at(instrId);
		else
			return CThostFtdcInstrumentField();
	}

	std::string AccountAndPositionMgr::getInstrumentList() const{
		std::string ret;
		for (auto item : m_instrument_dict){
			ret += (item.first + ", ");
		}
		return ret.substr(0, ret.size() - 1);
	}

	double AccountAndPositionMgr::getPosition(double& pos, Direction& direction, double& available) const{
		double money_long = 0.0; //THOST_FTDC_D_Buy
		double money_short = 0.0; // THOST_FTDC_D_Sell

		std::unique_lock<std::mutex> lk(m_mutex);

		for (auto item : m_tradeList_nonClosed_account_long){
			money_long += (item.Price * item.Volume * m_instrument_dict.at(item.InstrumentID).VolumeMultiple);// operator [] is not const, so that use at() instead
		}

		for (auto item : m_tradeList_notClosed_account_short){
			money_short += (item.Price * item.Volume * m_instrument_dict.at(item.InstrumentID).VolumeMultiple);
		}
		lk.unlock();

		pos = std::abs(money_long - money_short);
		direction = money_long > money_short ? AP::Long : AP::Short;
		available = m_accountInfo.Available;

		return pos;
	}

	int AccountAndPositionMgr::getPositionVolume(const std::string& instruID, Direction& todayDirection, int& todayPos, Direction& ydDirection, int& ydPos) const{
		if (m_tradeMessage_dict.find(instruID) != m_tradeMessage_dict.end()){
			int pos = m_tradeMessage_dict.at(instruID).Holding_long - m_tradeMessage_dict.at(instruID).Holding_short;
			int todayPos1 = m_tradeMessage_dict.at(instruID).TodayPosition_long - m_tradeMessage_dict.at(instruID).TodayPosition_short;
			int ydPos1 = m_tradeMessage_dict.at(instruID).YdPosition_long - m_tradeMessage_dict.at(instruID).YdPosition_short;
			todayDirection = todayPos > 0 ? AP::Long : AP::Short;
			ydDirection = ydPos > 0 ? AP::Long : AP::Short;
			todayPos = std::abs(todayPos1);
			ydPos = std::abs(ydPos1);
			return std::abs(pos);
		}
		else{
			return 0;
		}
	}
}

