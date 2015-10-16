#include "stdafx.h"
#include "AP_Mgr.h"
#include "Order.h"
#include <memory>
#include <sstream>

namespace AP{

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


	void AccountAndPositionMgr::update(const CThostFtdcTradingAccountField& info){
		memcpy(&m_accountInfo, &info, sizeof(CThostFtdcTradingAccountField));
	}

	void AccountAndPositionMgr::pushTodayNewTrade(const CThostFtdcTradeField& tradeField){
		m_newTrades.push_back(tradeField);
	}

	void AccountAndPositionMgr::pushTodayOrder(const CThostFtdcOrderField& orderField){
		m_orderlist.push_back(orderField);
	}

	std::string AccountAndPositionMgr::TodayOrderToString() const{
		return PrintUtils::ConvertOrderListToString(m_orderlist);
	}

	void AccountAndPositionMgr::pushTodayTrade(const CThostFtdcTradeField& tradeField){
		m_tradelist.push_back(tradeField);
	}

	std::string AccountAndPositionMgr::TodayTradeToString() const{
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

	std::string AccountAndPositionMgr::YesterdayUnClosedTradeToString(Direction direction){
		if (direction == Buy){
			return PrintUtils::ConvertTradeListToString(m_tradeList_nonClosed_account_long);
		}
		else if (direction == Sell){
			return PrintUtils::ConvertTradeListToString(m_tradeList_notClosed_account_short);
		}
		else
			return "";
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
			result << "���͹�˾����:" << iter->BrokerID << std::endl
				<< " Ͷ���ߴ���:" << iter->InvestorID << std::endl
				<< " �û�����:" << iter->UserID << std::endl
				<< " ��Լ����:" << iter->InstrumentID << std::endl
				<< " ��������:" << iter->Direction << std::endl
				<< " ��Ͽ�ƽ��־:" << iter->CombOffsetFlag << std::endl
				<< " �۸�:" << iter->LimitPrice << std::endl
				<< " ����:" << iter->VolumeTotalOriginal << std::endl
				<< " ��������:" << iter->OrderRef << std::endl
				<< " �ͻ�����:" << iter->ClientID << std::endl
				<< " ����״̬:" << iter->OrderStatus << std::endl
				<< " ί��ʱ��:" << iter->InsertTime << std::endl
				<< " �������:" << iter->OrderSysID << std::endl
				<< " GTD����:" << iter->GTDDate << std::endl
				<< " ������:" << iter->TradingDay << std::endl
				<< " ��������:" << iter->InsertDate << std::endl;
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
			result << "��Լ����:" << iter->InstrumentID << std::endl
				<< " �û�����:" << iter->UserID << std::endl
				<< " �ɽ����:" << iter->TradeID << std::endl
				<< " ��������:" << iter->Direction << std::endl
				<< " ��ƽ��־:" << iter->OffsetFlag << std::endl
				<< " Ͷ���ױ���־:" << iter->HedgeFlag << std::endl
				<< " �۸�:" << iter->Price << std::endl
				<< " ����:" << iter->Volume << std::endl
				<< " �ɽ�ʱ��:" << iter->TradeTime << std::endl
				<< " �ɽ�����:" << iter->TradeType << std::endl
				<< " �������:" << iter->OrderSysID << std::endl
				<< " ��������:" << iter->OrderRef << std::endl
				<< " ���ر������:" << iter->OrderLocalID << std::endl
				<< " ҵ��Ԫ:" << iter->BusinessUnit << std::endl
				<< " ���:" << iter->SequenceNo << std::endl
				<< " ���͹�˾�������:" << iter->BrokerOrderSeq << std::endl
				<< " �ɽ�ʱ��:" << iter->TradeDate << std::endl
				<< " ������:" << iter->TradingDay << std::endl;
		}
		result << "--------------------------------------------------" << std::endl;
		return result.str();
	}
}