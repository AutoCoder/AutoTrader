#ifndef ACCOUNT_MANAGER_H_
#define ACCOUNT_MANAGER_H_

#include "ThostFtdcTraderApi.h"
class IAccount;
class Order;

class CtpTradeSpi : public CThostFtdcTraderSpi
{
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

	bool IsFrontConnected() const{
		return m_isFrontConnected;
	}

	bool IsLogin() const {
		return m_islogin;
	}

	bool IsConfirmedSettlementInfo() const {
		return m_isConfirmSettlementInfo;
	}

	bool IsAccoutRefreshed() const {
		return m_isAccountFreshed;
	}

	void AddSubscriber(IAccount* pAccount){
		pAccountMgr = pAccount;
	}

private:
	double m_available; //可用资金
	double m_currMargin; //当前保证金总额
	double m_frozenCommission;//冻结手续费
	double m_frozenMargin;//冻结保证金
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
	bool m_isFrontConnected;
	bool m_islogin;
	bool m_isConfirmSettlementInfo;
	bool m_isAccountFreshed;
	IAccount* pAccountMgr;
};

#endif 