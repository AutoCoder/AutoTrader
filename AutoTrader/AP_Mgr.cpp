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