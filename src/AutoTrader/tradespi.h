#ifndef TRADE_SPI_H_
#define TRADE_SPI_H_

#include "ThostFtdcTraderApi.h"
//#include <future>
#include <mutex>
#include <condition_variable>
#include "crossplatform.h"
#include <functional>

class IPositionControl;
class Order;
namespace AP{
	class AccountDetailMgr;
};

extern std::condition_variable cv_md;

typedef std::function<void()>															InitedAccountCallback;
typedef std::function<void(CThostFtdcOrderField*)>										RtnOrderCallback;
typedef std::function<void(CThostFtdcTradeField*)>										RtnTradeCallback;
typedef std::function<void(CThostFtdcInputOrderActionField*, CThostFtdcRspInfoField*)>	CancelOrderCallback;

class CtpTradeSpi : public CThostFtdcTraderSpi
{

	class TradeThreadStateChangedHandler{
		//Sleep(1000) is for avoiding query frequency limit, otherwise the queryRequest will return -3
	public:
		TradeThreadStateChangedHandler(CtpTradeSpi* pTradeUserSpi)
			:m_TradeUserSpiPtr(pTradeUserSpi)
		{
		}

		//step 1
		void OnFrontConnected(){
			m_TradeUserSpiPtr->ReqUserLogin();
		}

		//step 2
		void OnLogined(){
			m_TradeUserSpiPtr->ReqSettlementInfoConfirm();
		}

		//step 3 
		void OnConfirmedSettlementInfo(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryOrder();
		}

		//step 4 : get the order list 
		void OnRspQryOrder(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTrade();
		}

		//step 5 : get the trade list
		//�����ѯ�ɽ���Ӧ,Ҫ���ֳ�������ʱ��һ�β�ѯ��֮��Ĳ�ѯ
		//����ֻ�ܲ�ѯ����ĳɽ�����ֲ��ܣ����Ի�Ҫ��ѯ�ֲ���ϸOnRspQryInvestorPositionDetail()
		void OnRspQryTrade(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPositionDetail();
		}

		//step 6: query account
		void OnRspQryInvestorPositionDetail(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTradingAccount();
		}

		//step 7:
		void OnRspQryTradingAccount(){
			//��ѯ���к�Լ�ĳֲ�
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPosition_all();
		}

		//step 8
		void OnRspQryInvestorPosition(){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInstrument_all();
		}

		//step 9
		void OnRspQryInstrument(){
			//invoke md thread
			//cv_md.notify_all();
			//send Account init finish action to fifo
		}

	private:
		CtpTradeSpi* m_TradeUserSpiPtr;
	};

public:
	CtpTradeSpi(CThostFtdcTraderApi* pUserApi,
		const char * brokerID, const char* userID, const char* password, const char* prodName, 
		AP::AccountDetailMgr& admgr, 
		InitedAccountCallback initFinishCallback,
		RtnOrderCallback onRtnOrderCallback,
		RtnTradeCallback onRtnTradeCallback,
		CancelOrderCallback OnRtnCancellOrderCallback);
	~CtpTradeSpi();

	virtual void OnFrontConnected();

	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ������Ӧ
	virtual void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯ�ɽ���Ӧ
	virtual void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///�����ѯͶ���ֲ߳���ϸ��Ӧ
	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);

	virtual void OnHeartBeatWarning(int nTimeLapse);

	virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

	virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
	//excute order queue

public:
	void ReqUserLogin();

	void ReqSettlementInfoConfirm();

	void ReqQryOrder();

	void ReqQryTrade();

	void ReqQryInvestorPositionDetail();

	void ReqQryInstrument(TThostFtdcInstrumentIDType instId);

	void ReqQryTradingAccount();

	void ReqQryInvestorPosition_all();

	void ReqQryInvestorPosition();

	void ReqQryInstrument_all();

	void ReqOrderInsert(Order ord);

	void ReqOrderAction(const CThostFtdcOrderField& order);// TThostFtdcSequenceNoType orderSeq, TThostFtdcExchangeIDType exchangeId, TThostFtdcOrderSysIDType orderSysId);

	void ForceClose();

	//��⣺�ڴ������µĽ���֮ǰ�������ύδ�ɽ��ı���������
	void CancelOrder(long long MDtime, int aliveDuration = 6, const std::string& instrumentId = "");

	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

private:
	TThostFtdcBrokerIDType m_brokerID;
	TThostFtdcUserIDType m_userID;
	TThostFtdcPasswordType m_password;
	TThostFtdcProductInfoType m_productName;
	TThostFtdcFrontIDType m_frontID;
	TThostFtdcSessionIDType m_sessionID;
	char m_orderRef[13];
	CThostFtdcTradingAccountField m_accountInfo;

	bool m_firstquery_order;//�Ƿ��״β�ѯ����
	bool m_firstquery_trade;//�Ƿ��״β�ѯ�ɽ�
	bool m_firstquery_Detail;//�Ƿ��״β�ѯ�ֲ���ϸ
	bool m_firstquery_TradingAccount;//�Ƿ��״β�ѯ�ʽ��˺�
	bool m_firstquery_Position;//�Ƿ��״β�ѯͶ���ֲ߳�
	bool m_firstquery_Instrument;//�Ƿ��״β�ѯ��Լ

private:
	InitedAccountCallback								m_initFinish_callback;
	RtnOrderCallback									m_OnRtnOrder_callback;
	RtnTradeCallback									m_OnRtnTrade_callback;
	CancelOrderCallback									m_OnCancelOrder_callback;
	AP::AccountDetailMgr&								m_account_detail_mgr;
	CThostFtdcTraderApi*								pUserApi;
	TradeThreadStateChangedHandler						m_stateChangeHandler;
};

#endif 