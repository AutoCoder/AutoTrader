#ifndef ACCOUNT_POSITION_H
#define ACCOUNT_POSITION_H

#include "stdafx.h"
#include "ThostFtdcUserApiStruct.h"
#include <mutex>
#include <vector>
#include <map>

class Order;

namespace AP{ // Account & Position

	struct STRATEGY_API TradeMessage
	{
		TradeMessage()
		{
			InstId = "";
			LastPrice = 0.0;
			PreSettlementPrice = 0.0;
			Holding_long = 0;
			Holding_short = 0;
			TodayPosition_long = 0;
			YdPosition_long = 0;
			TodayPosition_short = 0;
			YdPosition_short = 0;

			closeProfit_long = 0.0;
			OpenProfit_long = 0.0;
			closeProfit_short = 0.0;
			OpenProfit_short = 0.0;
		}


		std::string InstId;//合约代码
		double LastPrice;//最新价，时刻保存合约的最新价，平仓用
		double PreSettlementPrice;//上次结算价，对隔夜仓有时候要用，快期有用
		int Holding_long;//多单持仓量
		int Holding_short;//空单持仓量

		int TodayPosition_long;//多单今日持仓
		int YdPosition_long;//多单上日持仓

		int TodayPosition_short;//空单今日持仓
		int YdPosition_short;//空单上日持仓

		double closeProfit_long;//多单平仓盈亏
		double OpenProfit_long;//多单浮动盈亏

		double closeProfit_short;//空单平仓盈亏
		double OpenProfit_short;//空单浮动盈亏

	};

	enum Direction{
		Long = '0',
		Short = '1',
	};

	//class PrintUtils{
	//public:
	//	static std::string ConvertOrderListToString(const std::vector< CThostFtdcOrderField >& list);
	//	static std::string ConvertTradeListToString(const std::vector< CThostFtdcTradeField >& list);
	//};

	class STRATEGY_API AccountDetailMgr
	{
	public:
		AccountDetailMgr();
		virtual ~AccountDetailMgr();

		virtual void setAccountStatus(const CThostFtdcTradingAccountField& info);

		void pushTodayNewTrade(const CThostFtdcTradeField& tradeField);//Push the happening trade

		void pushTodayOrder(const CThostFtdcOrderField& orderField);

		void pushImmediateOrder(const CThostFtdcOrderField& orderField);

		const std::vector< CThostFtdcOrderField >& getAllOrders() const { return m_orderlist; }

		std::string todayOrderToString() const;

		size_t todayOrderCount() const { return m_orderlist.size();  }

		void pushTodayTrade(const CThostFtdcTradeField& tradeField);//Push the existed trade

		size_t todayTradeCount() const { return m_tradelist.size(); }

		std::string todayTradeToString() const;

		void pushYesterdayUnClosedTrade(const CThostFtdcTradeField& tradeField, Direction direction);

		std::string yesterdayUnClosedTradeToString(Direction direction);

		size_t yesterdayUnClosedTradeCount(Direction direction){ return direction == AP::Long ? m_tradeList_nonClosed_account_long.size() : m_tradeList_notClosed_account_short.size(); };

		void pushTradeMessage(const CThostFtdcInvestorPositionField& originalTradeStruct);

		double getCloseProfit();//平仓盈亏，所有合约一起算后的值，另外在m_trade_message_map有单独计算每个合约的平仓盈亏值

		double getOpenProfit();//浮动盈亏，所有合约一起算后的值，另外在m_trade_message_map有单独计算每个合约的浮动盈亏值

		void pushInstrumentStruct(const CThostFtdcInstrumentField& instru);

		CThostFtdcInstrumentField getInstrumentField(const std::string& instrId) const;

		std::string getInstrumentList() const;

		double getPosition(double& pos, Direction& direction, double& available) const;
		
		//return total volume, set TodayPosition & YdPosition by reference
		int getPositionVolume(const std::string& instruID, Direction& todayDirection, int& todayPos, Direction& ydDirection, int& ydPos) const;

		const std::map<std::string, AP::TradeMessage>& getAllPositionMap() const { return m_tradeMessage_dict; }

		//indicate that the Account init is finish, is ready for return account info back to client
		void finishInitialize();

	private:
		AccountDetailMgr(const AccountDetailMgr& mgr) = delete;
		AccountDetailMgr& operator = (const AccountDetailMgr& mgr) = delete;
		

	private:
		mutable std::mutex m_mutex; //sync GetPosition() between pushTodayOrder() & pushTodayTrade
		//TThostFtdcInstrumentIDType m_instrument;
		CThostFtdcTradingAccountField m_accountInfo;
		CThostFtdcInvestorPositionField m_positionInfo;

		std::vector< CThostFtdcOrderField > m_orderlist; //委托记录，全部合约
		std::vector< CThostFtdcTradeField > m_tradelist; //成交记录，全部合约
		std::vector< CThostFtdcTradeField > m_tradeList_nonClosed_account_long;//未平仓的多单成交记录,整个账户，全部合约
		std::vector< CThostFtdcTradeField > m_tradeList_notClosed_account_short;//未平仓的空单成交记录,整个账户，全部合约
		volatile bool m_isReady;

		std::map<std::string, TradeMessage> m_tradeMessage_dict;
		std::map<std::string, CThostFtdcInstrumentField> m_instrument_dict;

		std::vector< CThostFtdcTradeField > m_newTrades; //委托记录，全部合约
	};

	
}

//typedef AP::AccountDetailMgr AccountDetailMgr;

#endif