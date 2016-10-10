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

namespace PP{
	class PositionProfitMgr;
}

typedef std::function<void()>															InitedAccountCallback;
typedef std::function<void(CThostFtdcOrderField*)>										RtnOrderCallback;
typedef std::function<void(CThostFtdcTradeField*)>										RtnTradeCallback;
typedef PP::PositionProfitMgr PPMgr;

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

			m_TradeUserSpiPtr->InitializationFinished();
		}

	private:
		CtpTradeSpi* m_TradeUserSpiPtr;
	};

public:
	CtpTradeSpi(CThostFtdcTraderApi* pUserApi,
		const char * brokerID, const char* userID, const char* password, const char* prodName,
		PPMgr& ppmgr,
		InitedAccountCallback onInitedAccountCallback,
		RtnOrderCallback onRtnOrderCallback,
		RtnTradeCallback onRtnTradeCallback);
        virtual	~CtpTradeSpi();

		void ReqOrderInsert(Order ord);

		void ReqOrderAction(const CThostFtdcOrderField& order);// TThostFtdcSequenceNoType orderSeq, TThostFtdcExchangeIDType exchangeId, TThostFtdcOrderSysIDType orderSysId);

		//��⣺�ڴ������µĽ���֮ǰ�������ύδ�ɽ��ı���������
		void CancelOrder(long long MDtime, int aliveDuration = 6, const std::string& instrumentId = "");

		void ForceClose();

		void InitializationFinished();

private:

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

private:
	void ReqUserLogin();

	void ReqSettlementInfoConfirm();

	void ReqQryOrder();

	void ReqQryTrade();

	void ReqQryInvestorPositionDetail();

	void ReqQryInstrument(TThostFtdcInstrumentIDType instId);

	void ReqQryTradingAccount();

	void ReqQryInvestorPosition_all();

	void ReqQryInvestorPosition();

	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

private:
	CThostFtdcTraderApi*								pUserApi;
	TThostFtdcBrokerIDType								m_brokerID;
	TThostFtdcUserIDType								m_userID;
	TThostFtdcPasswordType								m_password;
	TThostFtdcProductInfoType							m_productName;
	TThostFtdcFrontIDType								m_frontID;
	TThostFtdcSessionIDType								m_sessionID;
	char												m_orderRef[13];
	
private:
	InitedAccountCallback                               m_OnInitedAccount_Callback;
	RtnOrderCallback									m_OnRtnOrder_callback;
	RtnTradeCallback									m_OnRtnTrade_callback;
	PPMgr&												m_ppmgr;
	
	TradeThreadStateChangedHandler						m_stateChangeHandler;
	int              									m_requestId;
};

#endif 
