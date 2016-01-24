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


		std::string InstId;//��Լ����
		double LastPrice;//���¼ۣ�ʱ�̱����Լ�����¼ۣ�ƽ����
		double PreSettlementPrice;//�ϴν���ۣ��Ը�ҹ����ʱ��Ҫ�ã���������
		int Holding_long;//�൥�ֲ���
		int Holding_short;//�յ��ֲ���

		int TodayPosition_long;//�൥���ճֲ�
		int YdPosition_long;//�൥���ճֲ�

		int TodayPosition_short;//�յ����ճֲ�
		int YdPosition_short;//�յ����ճֲ�

		double closeProfit_long;//�൥ƽ��ӯ��
		double OpenProfit_long;//�൥����ӯ��

		double closeProfit_short;//�յ�ƽ��ӯ��
		double OpenProfit_short;//�յ�����ӯ��

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

		double getCloseProfit();//ƽ��ӯ�������к�Լһ������ֵ��������m_trade_message_map�е�������ÿ����Լ��ƽ��ӯ��ֵ

		double getOpenProfit();//����ӯ�������к�Լһ������ֵ��������m_trade_message_map�е�������ÿ����Լ�ĸ���ӯ��ֵ

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

		std::vector< CThostFtdcOrderField > m_orderlist; //ί�м�¼��ȫ����Լ
		std::vector< CThostFtdcTradeField > m_tradelist; //�ɽ���¼��ȫ����Լ
		std::vector< CThostFtdcTradeField > m_tradeList_nonClosed_account_long;//δƽ�ֵĶ൥�ɽ���¼,�����˻���ȫ����Լ
		std::vector< CThostFtdcTradeField > m_tradeList_notClosed_account_short;//δƽ�ֵĿյ��ɽ���¼,�����˻���ȫ����Լ
		volatile bool m_isReady;

		std::map<std::string, TradeMessage> m_tradeMessage_dict;
		std::map<std::string, CThostFtdcInstrumentField> m_instrument_dict;

		std::vector< CThostFtdcTradeField > m_newTrades; //ί�м�¼��ȫ����Լ
	};

	
}

//typedef AP::AccountDetailMgr AccountDetailMgr;

#endif