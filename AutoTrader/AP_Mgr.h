#ifndef BASE_ACCOUNT_H
#define BASE_ACCOUNT_H

#include "ThostFtdcUserApiStruct.h"
//#include "IPositionControl.h"
#include <mutex>
#include <vector>

class Order;

namespace AP{

	enum Direction{
		Buy = '0',
		Sell = '1',
	};

	class PrintUtils{
	public:
		static std::string ConvertOrderListToString(const std::vector< CThostFtdcOrderField >& list);
		static std::string ConvertTradeListToString(const std::vector< CThostFtdcTradeField >& list);
	};

	class AccountAndPositionMgr //: public IPositionControl
	{
	public:
		AccountAndPositionMgr();
		virtual ~AccountAndPositionMgr();

		virtual void update(const CThostFtdcTradingAccountField& info);

		void pushTodayNewTrade(const CThostFtdcTradeField& tradeField);

		void pushTodayOrder(const CThostFtdcOrderField& orderField);

		std::string TodayOrderToString() const;

		size_t TodayOrderCount() const { return m_orderlist.size();  }

		void pushTodayTrade(const CThostFtdcTradeField& tradeField);

		size_t TodayTradeCount() const { return m_tradelist.size(); }

		std::string TodayTradeToString() const;

		void pushYesterdayUnClosedTrade(const CThostFtdcTradeField& tradeField, Direction direction);

		std::string YesterdayUnClosedTradeToString(Direction direction);

		long yesterdayUnClosedTradeCount(Direction direction){ return direction == AP::Buy ? m_tradeList_nonClosed_account_long.size() : m_tradeList_notClosed_account_short.size(); };

		double GetPosition(double& pos, Direction& direction, int& volume, double& available) const;

		virtual bool isReady() { return m_isReady; }

	private:
		AccountAndPositionMgr(const AccountAndPositionMgr& mgr) = delete;
		AccountAndPositionMgr& operator = (const AccountAndPositionMgr& mgr) = delete;

	private:
		mutable std::mutex m_mutex; //sync GetPosition() between pushTodayOrder() & pushTodayTrade
		//TThostFtdcInstrumentIDType m_instrument;
		CThostFtdcTradingAccountField m_accountInfo;
		CThostFtdcInvestorPositionField m_positionInfo;


		//以下这些字段全部是在程序启动时获得。 也就是 isReady() == true 以前
		std::vector< CThostFtdcOrderField > m_orderlist; //委托记录，全部合约
		std::vector< CThostFtdcTradeField > m_tradelist; //成交记录，全部合约
		std::vector< CThostFtdcTradeField > m_tradeList_nonClosed_account_long;//未平仓的多单成交记录,整个账户，全部合约
		std::vector< CThostFtdcTradeField > m_tradeList_notClosed_account_short;//未平仓的空单成交记录,整个账户，全部合约
		volatile bool m_isReady;

		//以下这些字段是 在isReady() == true 之后获得。
		std::vector< CThostFtdcTradeField > m_newTrades; //委托记录，全部合约
	};

	AccountAndPositionMgr& GetManager();
}



#endif