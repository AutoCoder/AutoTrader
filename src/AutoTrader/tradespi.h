#ifndef TRADE_SPI_H_
#define TRADE_SPI_H_

#include "ThostFtdcTraderApi.h"
#include <mutex>
#include <condition_variable>
#include "crossplatform.h"
#include <functional>
#include "AccountMgr.h"

class IPositionControl;
class Order;
namespace AP{
	class AccountDetailMgr;
};

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
		void OnLastRspQryOrder(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTrade();
		}

		//step 5 : get the trade list
		//�����ѯ�ɽ���Ӧ,Ҫ���ֳ�������ʱ��һ�β�ѯ��֮��Ĳ�ѯ
		//����ֻ�ܲ�ѯ����ĳɽ�����ֲ��ܣ����Ի�Ҫ��ѯ�ֲ���ϸOnRspQryInvestorPositionDetail()
		void OnLastRspQryTrade(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPositionDetail();
		}

		//step 6: query account
		void OnLastRspQryInvestorPositionDetail(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryTradingAccount();
		}

		//step 7:
		void OnLastRspQryTradingAccount(bool successful = true){
			//��ѯ���к�Լ�ĳֲ�
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInvestorPosition_all();
		}

		//step 8
		void OnLastRspQryInvestorPosition(bool successful = true){
			sleep(1000);
			m_TradeUserSpiPtr->ReqQryInstrument_all();
		}

		//step 9
		void OnLastRspQryInstrument(bool successful = true){
			//Todo: loop all related Instrument, query their marginrate CommissionRate
			for (auto instru : Account::Manager::Instance().Instruments()){
				sleep(1000);
				m_querying.store(true);
				m_TradeUserSpiPtr->ReqQryInstrumentMarginRate(instru.c_str());
				WaitQueryEnd();

				sleep(1000);
				m_querying.store(true);
				m_TradeUserSpiPtr->ReqQryInstrumentCommissionRate(instru.c_str());
				WaitQueryEnd();
			}

			m_TradeUserSpiPtr->InitializationFinished();
		}

	public:
		void WaitQueryEnd(){
			std::unique_lock<std::mutex> lk(m_mtx);
			m_con.wait(lk, [this]() -> bool { return !m_querying.load(); });
		}
		
		void NotifyQueryEnd(){
			m_querying.store(false);
			m_con.notify_all();
		}

	private:
		std::condition_variable m_con;
		std::atomic<bool>       m_querying;
		std::mutex              m_mtx;
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
        virtual	~CtpTradeSpi();

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

	//��Ӧ��ѯ��Լ��֤����
	void OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///��Ӧ��ѯ��Լ��������
	void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///��Ӧ��ѯ��Ȩ���׳ɱ�
	void OnRspQryOptionInstrTradeCost(CThostFtdcOptionInstrTradeCostField *pOptionInstrTradeCost, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///��Ӧ��ѯ��Ȩ��Լ������
	void OnRspQryOptionInstrCommRate(CThostFtdcOptionInstrCommRateField *pOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

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

	///�����ѯ��Լ��֤����
	void ReqQryInstrumentMarginRate(const char* instId);

	///�����ѯ��Լ��������
	void ReqQryInstrumentCommissionRate(const char* instId);

	///�����ѯ��Ȩ���׳ɱ�
	void ReqQryOptionInstrTradeCost(const char* instId);

	///�����ѯ��Ȩ��Լ������
	void ReqQryOptionInstrCommRate(const char* instId);

	void ForceClose();

	//��⣺�ڴ������µĽ���֮ǰ�������ύδ�ɽ��ı���������
	void CancelOrder(long long MDtime, int aliveDuration = 6, const std::string& instrumentId = "");

	void InitializationFinished();

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

	//bool m_firstquery_order;//�Ƿ��״β�ѯ����
	//bool m_firstquery_trade;//�Ƿ��״β�ѯ�ɽ�
	//bool m_firstquery_Detail;//�Ƿ��״β�ѯ�ֲ���ϸ
	//bool m_firstquery_TradingAccount;//�Ƿ��״β�ѯ�ʽ��˺�
	//bool m_firstquery_Position;//�Ƿ��״β�ѯͶ���ֲ߳�
	//bool m_firstquery_Instrument;//�Ƿ��״β�ѯ��Լ

private:
	InitedAccountCallback								m_initFinish_callback;
	RtnOrderCallback									m_OnRtnOrder_callback;
	RtnTradeCallback									m_OnRtnTrade_callback;
	CancelOrderCallback									m_OnCancelOrder_callback;
	AP::AccountDetailMgr&								m_account_detail_mgr;
	CThostFtdcTraderApi*								pUserApi;
	TradeThreadStateChangedHandler						m_stateChangeHandler;
	int              									m_requestId;
};

#endif 
