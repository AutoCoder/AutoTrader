#ifndef ACCOUNT_MANAGER_H_
#define ACCOUNT_MANAGER_H_

#include "ThostFtdcTraderApi.h"
class IAccount;
class Order;



class CtpTradeSpi : public CThostFtdcTraderSpi
{
	class TradeThreadStateChangedHandler{
	public:
		TradeThreadStateChangedHandler(CtpTradeSpi* pTradeUserSpi)
			:m_TradeUserSpiPtr(pTradeUserSpi)
		{
		}

		void OnFrontConnected(){
			m_TradeUserSpiPtr->ReqUserLogin();
		}

		void OnLogined(){
			m_TradeUserSpiPtr->ReqSettlementInfoConfirm();
		}

		void OnConfirmedSettlementInfo(){
			m_TradeUserSpiPtr->ReqQryTradingAccount();
			//m_TradeUserSpiPtr->ReqQryInvestorPosition();
		}

	private:
		CtpTradeSpi* m_TradeUserSpiPtr;
	};

public:
	CtpTradeSpi(CThostFtdcTraderApi* pUserApi, \
		const char * brokerID, \
		const char* userID, \
		const char* password, \
		const char* prodName);
	~CtpTradeSpi();

	virtual void OnFrontConnected();

	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

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

	void ReqQryInstrument(TThostFtdcInstrumentIDType instId);

	void ReqQryTradingAccount();

	void ReqQryInvestorPosition();//(TThostFtdcInstrumentIDType instId);

	void ReqOrderInsert(Order ord);

	void ReqOrderAction(TThostFtdcSequenceNoType orderSeq);

	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	//bool IsFrontConnected() const{
	//	return m_isFrontConnected;
	//}

	//bool IsLogin() const {
	//	return m_islogin;
	//}

	//bool IsConfirmedSettlementInfo() const {
	//	return m_isConfirmSettlementInfo;
	//}

	//bool IsAccoutRefreshed() const {
	//	return m_isAccountFreshed;
	//}

	void AddSubscriber(IAccount* pAccount){
		pAccountMgr = pAccount;
	}

private:
	TThostFtdcBrokerIDType m_brokerID;
	TThostFtdcUserIDType m_userID;
	TThostFtdcPasswordType m_password;
	TThostFtdcProductInfoType m_productName;
	TThostFtdcFrontIDType m_frontID;
	TThostFtdcSessionIDType m_sessionID;
	char m_orderRef[13];
	CThostFtdcTradingAccountField m_accountInfo;
	const double ganggan = 0.2;
private:
	CThostFtdcTraderApi* pUserApi;
	//bool m_isFrontConnected;
	//bool m_islogin;
	//bool m_isConfirmSettlementInfo;
	//bool m_isAccountFreshed;
	IAccount* pAccountMgr;
	TradeThreadStateChangedHandler m_stateChangeHandler;
};

#endif 