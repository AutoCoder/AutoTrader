#include "stdafx.h"
#include "AP_Mgr.h"
#include "Order.h"
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

	AccountAndPositionMgr& GetManager(){
		static AccountAndPositionMgr mgr;
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

	std::string AccountAndPositionMgr::todayOrderToString() const{
		return PrintUtils::ConvertOrderListToString(m_orderlist);
	}

	void AccountAndPositionMgr::pushTodayTrade(const CThostFtdcTradeField& tradeField){
		m_tradelist.push_back(tradeField);
	}

	std::string AccountAndPositionMgr::todayTradeToString() const{
		return PrintUtils::ConvertTradeListToString(m_tradelist);
	}

	void AccountAndPositionMgr::pushYesterdayUnClosedTrade(const CThostFtdcTradeField& tradeField, Direction direction){
		if (direction == Buy){
			m_tradeList_nonClosed_account_long.push_back(tradeField);
		}
		else if(direction == Sell){
			m_tradeList_notClosed_account_short.push_back(tradeField);
		}
	}

	std::string AccountAndPositionMgr::yesterdayUnClosedTradeToString(Direction direction){
		if (direction == Buy){
			return PrintUtils::ConvertTradeListToString(m_tradeList_nonClosed_account_long);
		}
		else if (direction == Sell){
			return PrintUtils::ConvertTradeListToString(m_tradeList_notClosed_account_short);
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

	std::string AccountAndPositionMgr::getInstrumentList() const{
		std::string ret;
		for (auto item : m_instrument_dict){
			ret += (item.first + ", ");
		}
		return ret.substr(0, ret.size() - 1);
	}

	double AccountAndPositionMgr::GetPosition(double& pos, Direction& direction, int& volume, double& available) const{
		double amount_buy = 0.0; //THOST_FTDC_D_Buy
		int volume_buy = 0;
		double amount_sell = 0.0; // THOST_FTDC_D_Sell
		int volume_sell = 0;
		
		std::unique_lock<std::mutex> lk(m_mutex);
		//for (auto item : m_PositionList){

		//	Instrument::InformationMgr& posMgr = Instrument::GetManager();
		//	int multiple = posMgr.GetVolumeMultiple(item.InstrumentID);
		//	if (item.Direction == THOST_FTDC_D_Buy){
		//		amount_buy += (item.Price * item.Volume * multiple);
		//		volume_buy += item.Volume;
		//	}
		//	else if (item.Direction == THOST_FTDC_D_Sell){
		//		amount_sell += (item.Price * item.Volume * multiple);
		//		volume_sell += item.Volume;
		//	}
		//	else{
		//		assert(false);
		//	}
		//}
		lk.unlock();
		direction = amount_buy > amount_sell ? AP::Buy : AP::Sell;
		pos = amount_buy > amount_sell ? (amount_buy - amount_sell) : (amount_sell - amount_buy);
		volume = volume_buy > volume_sell ? (volume_buy - volume_sell) : (volume_sell - volume_buy);
		available = 200.0;
		return pos;
	}


	std::string PrintUtils::ConvertOrderListToString(const std::vector< CThostFtdcOrderField >& list){
		if (list.empty())
			return "";

		std::stringstream result;
		result << std::endl << "------------------------------------------------" << std::endl;

		for (auto iter = list.begin(); iter != list.end(); iter++){
			result << "经纪公司代码:" << iter->BrokerID << std::endl
				<< " 投资者代码:" << iter->InvestorID << std::endl
				<< " 用户代码:" << iter->UserID << std::endl
				<< " 合约代码:" << iter->InstrumentID << std::endl
				<< " 买卖方向:" << iter->Direction << std::endl
				<< " 组合开平标志:" << iter->CombOffsetFlag << std::endl
				<< " 价格:" << iter->LimitPrice << std::endl
				<< " 数量:" << iter->VolumeTotalOriginal << std::endl
				<< " 报单引用:" << iter->OrderRef << std::endl
				<< " 客户代码:" << iter->ClientID << std::endl
				<< " 报单状态:" << iter->OrderStatus << std::endl
				<< " 委托时间:" << iter->InsertTime << std::endl
				<< " 报单编号:" << iter->OrderSysID << std::endl
				<< " GTD日期:" << iter->GTDDate << std::endl
				<< " 交易日:" << iter->TradingDay << std::endl
				<< " 报单日期:" << iter->InsertDate << std::endl;
		}
		result << "--------------------------------------------------" << std::endl;
		return result.str();
	}

	std::string PrintUtils::ConvertTradeListToString(const std::vector< CThostFtdcTradeField >& list){
		if (list.empty())
			return "";

		std::stringstream result;
		result << std::endl << "------------------------------------------------" << std::endl;

		for (auto iter = list.begin(); iter != list.end(); iter++){
			result << "合约代码:" << iter->InstrumentID << std::endl
				<< " 用户代码:" << iter->UserID << std::endl
				<< " 成交编号:" << iter->TradeID << std::endl
				<< " 买卖方向:" << iter->Direction << std::endl
				<< " 开平标志:" << iter->OffsetFlag << std::endl
				<< " 投机套保标志:" << iter->HedgeFlag << std::endl
				<< " 价格:" << iter->Price << std::endl
				<< " 数量:" << iter->Volume << std::endl
				<< " 成交时间:" << iter->TradeTime << std::endl
				<< " 成交类型:" << iter->TradeType << std::endl
				<< " 报单编号:" << iter->OrderSysID << std::endl
				<< " 报单引用:" << iter->OrderRef << std::endl
				<< " 本地报单编号:" << iter->OrderLocalID << std::endl
				<< " 业务单元:" << iter->BusinessUnit << std::endl
				<< " 序号:" << iter->SequenceNo << std::endl
				<< " 经纪公司报单编号:" << iter->BrokerOrderSeq << std::endl
				<< " 成交时期:" << iter->TradeDate << std::endl
				<< " 交易日:" << iter->TradingDay << std::endl;
		}
		result << "--------------------------------------------------" << std::endl;
		return result.str();
	}
}