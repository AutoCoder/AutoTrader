#include "stdafx.h"
#include "mdspi.h"
#include "TickWrapper.h"
#include "RealTimeDataProcessorPool.h"
#include "spdlog/spdlog.h"
#include "CommonUtils.h"

#include <iostream>
#include <condition_variable>
#include <atomic>

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

extern int requestId;  
extern std::condition_variable cv_md;
extern std::condition_variable cv_trade;
extern std::atomic<bool> g_quit;

namespace {


	void TryTerminate(const char * time){
		if (!g_quit && /*strcmp(time, "10:55:00") == 0){ //*/CommonUtils::IsMarketingTime(time) == false){
			g_quit = true;
			cv_md.notify_all();
			cv_trade.notify_all();
		}
	}
}


CtpMdSpi::CtpMdSpi(CThostFtdcMdApi* api, const std::vector<std::string>& instruments, const std::string& brokerId, const std::string& userID, const std::string& pw)
	: pUserApi(api)
	, m_stateChangeHandler(this, instruments, brokerId, userID, pw)
{
}

void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
  IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
	SYNC_PRINT << __FUNCTION__ << " reason=" << nReason;
}
		
void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	SYNC_PRINT << __FUNCTION__ << " TimerLapse = " << nTimeLapse;
}

void CtpMdSpi::OnFrontConnected()
{
	SYNC_PRINT << "[Md] Response | connected...";
	m_stateChangeHandler.OnFrontConnected();
}

bool CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType appId, TThostFtdcUserIDType userId, TThostFtdcPasswordType passwd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.UserID, userId);
	strcpy(req.Password, passwd);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
	SYNC_PRINT << "[Md] Request | send login request ..." << ((ret == 0) ? "success" : "fail");
  return (ret == 0);
  //SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
		SYNC_PRINT << "[Md] Response | login successfully...CurrentDate:" <<pRspUserLogin->TradingDay;
		m_stateChangeHandler.OnLogined();
	}
	//if (bIsLast) SetEvent(g_hEvent);
}
void CtpMdSpi::SubscribeMarketData(const std::vector<std::string>& instIdList){
	size_t len = instIdList.size();
	char** pInstId = new char*[len];
	for (size_t i = 0; i<len; i++)  
		pInstId[i] = const_cast<char*>(instIdList[i].c_str());

	int ret = pUserApi->SubscribeMarketData(pInstId, len);
	SYNC_PRINT << "[Md] Request | send md subscribe request... " << ((ret == 0) ? "success" : "fail");
}

void CtpMdSpi::OnRspSubMarketData(
         CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	SYNC_PRINT << "[Md] Response | [OnRspSubMarketData] : " << ((pRspInfo->ErrorID == 0) ? "success" : "fail") << "; DetailInfo : " << pRspInfo->ErrorMsg;
  //if(bIsLast)  SetEvent(g_hEvent);
	if (pRspInfo->ErrorID != 0){
		//m_isSubscribed = true;
		m_stateChangeHandler.OnLogined();
	}
}

void CtpMdSpi::OnRspUnSubMarketData(
             CThostFtdcSpecificInstrumentField *pSpecificInstrument,
             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	SYNC_PRINT << "[Md] Response | [OnRspUnSubMarketData] : " << ((pRspInfo->ErrorID == 0) ? "success" : "fail") << "; DetailInfo : " << pRspInfo->ErrorMsg;
  //if(bIsLast)  SetEvent(g_hEvent);
	
	if (pRspInfo->ErrorID == 0){
		//m_isSubscribed = false;
	}
}

void CtpMdSpi::OnRtnDepthMarketData(
             CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//SYNC_PRINT
	//<< "\n TradingDay: " << pDepthMarketData->TradingDay
	//<< "\n InstrumentID: " << pDepthMarketData->InstrumentID
	//<< "\n ExchangeID: " << pDepthMarketData->ExchangeID
	//<< "\n ExchangeInstID: " << pDepthMarketData->ExchangeInstID
	//<< "\n LastPrice: " << pDepthMarketData->LastPrice
	//<< "\n PreSettlementPrice: " << pDepthMarketData->PreSettlementPrice
	//<< "\n PreClosePrice: " << pDepthMarketData->PreClosePrice
	//<< "\n PreOpenInterest: " << pDepthMarketData->PreOpenInterest
	//<< "\n OpenPrice: " << pDepthMarketData->OpenPrice
	//<< "\n HighestPrice: " << pDepthMarketData->HighestPrice
	//<< "\n LowestPrice: " << pDepthMarketData->LowestPrice
	//<< "\n Volume: " << pDepthMarketData->Volume
	//<< "\n Turnover: " << pDepthMarketData->Turnover
	//<< "\n OpenInterest: " << pDepthMarketData->OpenInterest
	//<< "\n ClosePrice: " << pDepthMarketData->ClosePrice
	//<< "\n SettlementPrice: " << pDepthMarketData->SettlementPrice
	//<< "\n UpperLimitPrice: " << pDepthMarketData->UpperLimitPrice
	//<< "\n LowerLimitPrice:" << pDepthMarketData->LowerLimitPrice
	//<< "\n PreDelta: " << pDepthMarketData->PreDelta
	//<< "\n CurrDelta: " << pDepthMarketData->CurrDelta
	//<< "\n UpdateTime: " << pDepthMarketData->UpdateTime
	//<< "\n UpdateMillisec: " << pDepthMarketData->UpdateMillisec
	//<< "\n BidPrice1: " << pDepthMarketData->BidPrice1
	//<< "\n BidVolume1: " << pDepthMarketData->BidVolume1
	//<< "\n AskVolume5: " << pDepthMarketData->AskVolume5
	//<< "\n AveragePrice: " << pDepthMarketData->AveragePrice
	//<< "\n ActionDay: " << pDepthMarketData->ActionDay;

	//Important Node:
	//1) must create the local variable "pool" here, otherwise it will not call destruction fucntion when exit(0)
	//2) can't define a local RealTimeDataProcessor variable here, otherwise it will plus the ref-count, so that it will not call destruction fucntion when exit(0)
	auto pool = RealTimeDataProcessorPool::getInstance();
	TickWrapper tem(pDepthMarketData);
	pool->AppendRealTimeData(tem);

	TryTerminate(pDepthMarketData->UpdateTime);
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
	  SYNC_PRINT << "[Md] Response | " << pRspInfo->ErrorMsg;
  }
  return ret;
}