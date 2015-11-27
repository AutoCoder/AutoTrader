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

		//һ�����ֱ��� �ֶ�γɽ��� ���ж�εĳɽ�֪ͨ����ÿ�γɽ���Ŷ���һ�� 
		//ͨ��TradeID���ų��ظ�trade��ȷ���ɽ���¼���ظ�
		auto tradeIter = std::find_if(m_tradelist.begin(), m_tradelist.end(), IsSameTrade(tradeField));
		if (tradeIter != m_tradelist.end()){
			*tradeIter = tradeField;
		}
		else{
			m_tradelist.push_back(tradeField);
		}

		int close_num_account_long = 0;//ƽ�ֵĶ൥����������еĻ�
		int close_num_account_short = 0;//ƽ�ֵĿյ�����������еĻ�


		//���ǿ��ֵ����򱣴浽tradeList_notClosed_account_long��tradeList_notClosed_account_short

		if (tradeField.OffsetFlag == '0')//����
		{
			if (tradeField.Direction == '0')//�൥
			{
				//�൥�ֱֲ����Ӽ�¼
				m_tradeList_nonClosed_account_long.push_back(tradeField);
				//�൥�ֲ���
				m_tradeMessage_dict[tradeField.InstrumentID].Holding_long = m_tradeMessage_dict[tradeField.InstrumentID].Holding_long + tradeField.Volume;
				//�൥���ճֲ�
				m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long + tradeField.Volume;
			}
			else if (tradeField.Direction == '1')//�յ�
			{
				//�յ��ֱֲ����Ӽ�¼
				m_tradeList_notClosed_account_short.push_back(tradeField);
				//�յ��ֲ���
				m_tradeMessage_dict[tradeField.InstrumentID].Holding_short = m_tradeMessage_dict[tradeField.InstrumentID].Holding_short + tradeField.Volume;
				//�յ����ճֲ�
				m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short + tradeField.Volume;
			}
		}
		//ƽ�ֵ���ɾ����¼
		else if (tradeField.OffsetFlag == '1' || tradeField.OffsetFlag == '3')//1��ƽ�֣�3��ƽ��
		{
			if (tradeField.Direction == '1')//������ʾƽ��,����ֺͽ��ʱ����ʱ��˳����ƽ���
			{
				close_num_account_long = tradeField.Volume;

				for (auto iter = m_tradeList_nonClosed_account_long.begin(); iter != m_tradeList_nonClosed_account_long.end(); iter++)
				{
					if (strcmp(tradeField.InstrumentID, iter->InstrumentID) == 0)
					{
						if (iter->Volume < close_num_account_long)//û�к��Ե�tradeList_notClosed������Ϊ0�ĵ��ӣ�����Ӱ�������
						{
							close_num_account_long -= iter->Volume;
							//ƽ��ӯ��
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long + (tradeField.Price - iter->Price) * iter->Volume * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							iter->Volume = 0;
						}
						else if (iter->Volume == close_num_account_long)
						{
							iter->Volume = 0;
							//ƽ��ӯ��
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long + (tradeField.Price - iter->Price) * close_num_account_long * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							break;
						}
						else if (iter->Volume > close_num_account_long)
						{
							iter->Volume = iter->Volume - close_num_account_long;
							//ƽ��ӯ��
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_long + (tradeField.Price - iter->Price) * close_num_account_long * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							break;
						}
					}
				}

				//�൥�ֲ���
				m_tradeMessage_dict[tradeField.InstrumentID].Holding_long = m_tradeMessage_dict[tradeField.InstrumentID].Holding_long - tradeField.Volume;
				//��ֲֳ������������Ҫ���������ͷ�������


				//��������������ֻ����������Ч
				if (tradeField.OffsetFlag == '1')
					m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long = m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long - tradeField.Volume;//���
				else if (tradeField.OffsetFlag == '3')
					m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long - tradeField.Volume;//���


				//������5�֣����1��ƽ�ֶ��Ƿ�'1'������ƽ��2�֣����������-1����ֻ���5�֣�ʵ��Ӧ���ǽ��5-1�����0
				//3����֣�5�ֽ�֣���'1'ƽ����4��,���������-1����ֻ���5�֣�ʵ��Ӧ���ǽ��5-1�����0
				if (m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long < 0)
				{
					m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_long + m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long;
					m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_long = 0;
				}
			}
			else if (tradeField.Direction == '0')//ƽ��
			{
				close_num_account_short = tradeField.Volume;

				for (auto iter = m_tradeList_notClosed_account_short.begin(); iter != m_tradeList_notClosed_account_short.end(); iter++)
				{
					if (strcmp(tradeField.InstrumentID, iter->InstrumentID) == 0)
					{
						if (iter->Volume < close_num_account_short)//û�к��Ե�tradeList_notClosed������Ϊ0�ĵ��ӣ�����Ӱ�������
						{
							close_num_account_short -= iter->Volume;
							//ƽ��ӯ��
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short + (iter->Price - tradeField.Price) * iter->Volume * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							iter->Volume = 0;
						}

						else if (iter->Volume == close_num_account_short)
						{
							iter->Volume = 0;
							//ƽ��ӯ��
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short + (iter->Price - tradeField.Price) * close_num_account_short * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							break;
						}

						else if (iter->Volume > close_num_account_short)
						{
							iter->Volume = iter->Volume - close_num_account_short;
							//ƽ��ӯ��
							m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short = m_tradeMessage_dict[tradeField.InstrumentID].closeProfit_short + (iter->Price - tradeField.Price) * close_num_account_short * m_instrument_dict[tradeField.InstrumentID].VolumeMultiple;
							break;
						}

					}

				}

				//�յ��ֲ���
				m_tradeMessage_dict[tradeField.InstrumentID].Holding_short = m_tradeMessage_dict[tradeField.InstrumentID].Holding_short - tradeField.Volume;

				//�յ����ճֲ�
				//m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short = m_trade_message_map[trade_account->InstrumentID]->TodayPosition_short - trade_account->Volume;

				//��������������ֻ����������Ч
				if (tradeField.OffsetFlag == '1')
					m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_short = m_tradeMessage_dict[tradeField.InstrumentID].YdPosition_short - tradeField.Volume;//���
				else if (tradeField.OffsetFlag == '3')
					m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short = m_tradeMessage_dict[tradeField.InstrumentID].TodayPosition_short - tradeField.Volume;//���


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

		if (originalTradeStruct.PosiDirection == '2') //�൥
		{
			//�൥�ֲ���
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].Holding_long = originalTradeStruct.Position;
			//�൥���
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].TodayPosition_long = originalTradeStruct.TodayPosition;
			//�൥��� = �൥�ֲ��� - �൥���
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].YdPosition_long = originalTradeStruct.Position - originalTradeStruct.TodayPosition;
			//�൥ƽ��ӯ��
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].closeProfit_long = originalTradeStruct.CloseProfit;
			//�൥����ӯ��(��ʵ�ǳֲ�ӯ������������)
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].OpenProfit_long = originalTradeStruct.PositionProfit;
		}
		else if (originalTradeStruct.PosiDirection == '3') // �յ�
		{
			//�յ��ֲ���
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].Holding_short = originalTradeStruct.Position;
			//�յ����
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].TodayPosition_short = originalTradeStruct.TodayPosition;
			//�յ���� = �յ��ֲ��� - �յ����
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].YdPosition_short = originalTradeStruct.Position - m_tradeMessage_dict[originalTradeStruct.InstrumentID].TodayPosition_short;
			//�յ�ƽ��ӯ��
			m_tradeMessage_dict[originalTradeStruct.InstrumentID].closeProfit_short = originalTradeStruct.CloseProfit;
			//�յ�����ӯ��
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

