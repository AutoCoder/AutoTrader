#ifndef MD_SPI_H_
#define MD_SPI_H_
//#pragma once

#ifdef _DEBUG
#define QueueSize 1
#else
#define QueueSize 1000
#endif

#include <queue>
#include <vector>
#include "ThostFtdcMdApi.h"
#include "Strategy.h"
#include "ThostFtdcDepthMDFieldWrapper.h"

class CtpMdSpi : public CThostFtdcMdSpi
{
public:
  CtpMdSpi(CThostFtdcMdApi* api):pUserApi(api){};

	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);
		
	virtual void OnHeartBeatWarning(int nTimeLapse);

	virtual void OnFrontConnected();
	
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

public:
	bool ReqUserLogin(TThostFtdcBrokerIDType	appId,
	        TThostFtdcUserIDType	userId,	TThostFtdcPasswordType	passwd);
	void SubscribeMarketData(char* instIdList);
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);

	bool AppendRealTimeData(const CThostFtdcDepthMDFieldWrapper& info);
private:
  CThostFtdcMdApi* pUserApi;
  std::queue<CThostFtdcDepthMDFieldWrapper> m_DataSeq;
  std::vector<Strategy> m_strategies;
};

#endif