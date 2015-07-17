#include "stdafx.h"
#include "mdspi.h"
#include <iostream>
#include "TickWrapper.h"
#include "RealTimeDataProcessorPool.h"
#include <condition_variable>
#include <atomic>
#include "spdlog/spdlog.h"
#include "CommonUtils.h"

#pragma warning(disable : 4996)

extern int requestId;  
extern std::condition_variable cv_md;
extern std::condition_variable cv_trade;
extern std::atomic<bool> g_quit;

namespace {


	void TryTerminate(const char * time){
		if (!g_quit && strcmp(time, "10:55:00") == 0){ //CommonUtils::IsMarketingTime(time) == false){
			g_quit = true;
			cv_md.notify_all();
			cv_trade.notify_all();
		}
	}
}


CtpMdSpi::CtpMdSpi(CThostFtdcMdApi* api) 
	: pUserApi(api)
	, m_isSubscribed(false)
	, m_isFrontConnected(false)
	, m_isLogin(false)
{
	
}

void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
  IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
	spdlog::get("console")->info() << __FUNCTION__ << " reason=" << nReason;
	m_isFrontConnected = false;
	m_isLogin = false;
	m_isSubscribed = false;
}
		
void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	spdlog::get("console")->info() << __FUNCTION__ << " TimerLapse = " << nTimeLapse;
}

void CtpMdSpi::OnFrontConnected()
{
	spdlog::get("console")->info() << "[MD Thread] Response | connected...";
	m_isFrontConnected = true;
	cv_md.notify_all();
}

bool CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType appId, TThostFtdcUserIDType userId, TThostFtdcPasswordType passwd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.UserID, userId);
	strcpy(req.Password, passwd);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
	spdlog::get("console")->info() << "[MD Thread] Request | send login request ..." << ((ret == 0) ? "success" : "fail");
  return (ret == 0);
  //SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
		spdlog::get("console")->info() << "[MD Thread] Response | login successfully...CurrentDate:" <<pRspUserLogin->TradingDay;
		m_isLogin = true;
		cv_md.notify_all();
	}
	//if (bIsLast) SetEvent(g_hEvent);
}

void CtpMdSpi::SubscribeMarketData(char* instIdList)
{
	std::vector<char*> list;
	char *token = strtok(instIdList, ",");
	while( token != NULL ){
		list.push_back(token); 
		token = strtok(NULL, ",");
	}
	unsigned int len = list.size();
	char** pInstId = new char* [len];  
	for(unsigned int i=0; i<len;i++)  pInstId[i]=list[i]; 

	int ret=pUserApi->SubscribeMarketData(pInstId, len);
	spdlog::get("console")->info() << "[MD Thread] Request | send md subscribe request... " << ((ret == 0) ? "success" : "fail");
}

void CtpMdSpi::OnRspSubMarketData(
         CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	spdlog::get("console")->info() << "[MD Thread] Response | [OnRspSubMarketData] : " << ((pRspInfo->ErrorID == 0) ? "success" : "fail") << "; DetailInfo : " << pRspInfo->ErrorMsg;
  //if(bIsLast)  SetEvent(g_hEvent);
	if (pRspInfo->ErrorID == 0){
		m_isSubscribed = true;
	}
}

void CtpMdSpi::OnRspUnSubMarketData(
             CThostFtdcSpecificInstrumentField *pSpecificInstrument,
             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	spdlog::get("console")->info() << "[MD Thread] Response | [OnRspUnSubMarketData] : " << ((pRspInfo->ErrorID == 0) ? "success" : "fail") << "; DetailInfo : " << pRspInfo->ErrorMsg;
  //if(bIsLast)  SetEvent(g_hEvent);
	
	if (pRspInfo->ErrorID == 0){
		m_isSubscribed = false;
	}
}

void CtpMdSpi::OnRtnDepthMarketData(
             CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	//spdlog::get("console")->info()
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
	pool->GenRealTimeDataProcessor(pDepthMarketData->InstrumentID)->AppendRealTimeData(TickWrapper(pDepthMarketData));

	TryTerminate(pDepthMarketData->UpdateTime);
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
	  spdlog::get("console")->info() << "[MD Thread] Response | " << pRspInfo->ErrorMsg;
  }
  return ret;
}