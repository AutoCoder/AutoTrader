#pragma once
#include <iostream>

//#include "ThostFtdcTraderApi.h"
class CThostFtdcTraderApi;

class AccountMangerSpi : public CThostFtdcTraderSpi
{
public:
	AccountMangerSpi(CThostFtdcTraderApi* pUserApi, \
		const char * brokerID, \
		const char* userID, \
		const char* password);
	~AccountMangerSpi();

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

	void ReqQryInvestorPosition(TThostFtdcInstrumentIDType instId);

	void ReqOrderInsert(TThostFtdcInstrumentIDType instId,
		TThostFtdcDirectionType dir, TThostFtdcCombOffsetFlagType kpp,
		TThostFtdcPriceType price, TThostFtdcVolumeType vol);

	void ReqOrderAction(TThostFtdcSequenceNoType orderSeq);

	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	void ExcuteOrderQueue();

private:
	double m_availCash;
	double m_totalFund;
	double m_position;
	TThostFtdcBrokerIDType m_brokerID;
	TThostFtdcUserIDType m_userID;
	TThostFtdcPasswordType m_password;
	TThostFtdcFrontIDType m_frontID;
	TThostFtdcSessionIDType m_sessionID;
	char m_orderRef[13];
	CThostFtdcTradingAccountField m_accountInfo;
private:
	CThostFtdcTraderApi* pUserApi;
};

