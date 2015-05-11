#ifndef MD_SPI_H_
#define MD_SPI_H_

#include "ThostFtdcMdApi.h"
#include "windows.h"

class CtpMdSpi : public CThostFtdcMdSpi
{
public:
	CtpMdSpi(CThostFtdcMdApi* api);

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
	bool IsSubscribed(){ return m_isSubscribed; }
	bool IsFrontConnected() { return m_isFrontConnected; }
	bool IsLogin() { return m_isLogin; }
private:
  CThostFtdcMdApi* pUserApi;
  bool m_isSubscribed;
  bool m_isLogin;
  bool m_isFrontConnected;
};

#endif