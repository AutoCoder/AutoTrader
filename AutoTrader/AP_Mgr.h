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


		//������Щ�ֶ�ȫ�����ڳ�������ʱ��á� Ҳ���� isReady() == true ��ǰ
		std::vector< CThostFtdcOrderField > m_orderlist; //ί�м�¼��ȫ����Լ
		std::vector< CThostFtdcTradeField > m_tradelist; //�ɽ���¼��ȫ����Լ
		std::vector< CThostFtdcTradeField > m_tradeList_nonClosed_account_long;//δƽ�ֵĶ൥�ɽ���¼,�����˻���ȫ����Լ
		std::vector< CThostFtdcTradeField > m_tradeList_notClosed_account_short;//δƽ�ֵĿյ��ɽ���¼,�����˻���ȫ����Լ
		volatile bool m_isReady;

		//������Щ�ֶ��� ��isReady() == true ֮���á�
		std::vector< CThostFtdcTradeField > m_newTrades; //ί�м�¼��ȫ����Լ
	};

	AccountAndPositionMgr& GetManager();
}



#endif