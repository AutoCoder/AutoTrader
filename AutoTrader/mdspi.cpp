#include "stdafx.h"
#include "mdspi.h"
#include <iostream>
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "RealTimeDataProcessorPool.h"

using namespace std;
#pragma warning(disable : 4996)

extern int requestId;  
extern HANDLE g_hEvent;

namespace {
	void TryTerminate(const char * time){
		if (0 == strcmp(time, "14:58:00"))
			exit(0);
	}
}


CtpMdSpi::CtpMdSpi(CThostFtdcMdApi* api) 
	:pUserApi(api)
{
	auto pool = RealTimeDataProcessorPool::getInstance();
}

void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
  IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
	cerr << __FUNCTION__
    << " reason=" << nReason << endl;
}
		
void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
	cerr << __FUNCTION__
    << " TimerLapse = " << nTimeLapse << endl;
}

void CtpMdSpi::OnFrontConnected()
{
	cerr << __FUNCTION__ << endl;
	SetEvent(g_hEvent);
}

bool CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType appId, TThostFtdcUserIDType userId, TThostFtdcPasswordType passwd)
{
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(req.UserID, userId);
	strcpy(req.Password, passwd);
	int ret = pUserApi->ReqUserLogin(&req, ++requestId);
  cerr<<" Request | send login request ..."<<((ret == 0) ? "success" : "fail") << endl;	
  return (ret == 0);
  //SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
    cerr<<" Response | login successfully...CurrentDate:"
      <<pRspUserLogin->TradingDay<<endl;
	}
	if (bIsLast) SetEvent(g_hEvent);
}

void CtpMdSpi::SubscribeMarketData(char* instIdList)
{
  vector<char*> list;
  char *token = strtok(instIdList, ",");
  while( token != NULL ){
    list.push_back(token); 
    token = strtok(NULL, ",");
  }
  unsigned int len = list.size();
  char** pInstId = new char* [len];  
  for(unsigned int i=0; i<len;i++)  pInstId[i]=list[i]; 
	int ret=pUserApi->SubscribeMarketData(pInstId, len);
	cerr << " Request | send md subscribe request... " << ((ret == 0) ? "success" : "fail") << endl;
  //SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspSubMarketData(
         CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << " Response | [OnRspSubMarketData] : " << ((pRspInfo->ErrorID == 0) ? "success" : "fail") << "; DetailInfo : " << pRspInfo->ErrorMsg << endl;
  //if(bIsLast)  SetEvent(g_hEvent);
	SetEvent(g_hEvent);
}

void CtpMdSpi::OnRspUnSubMarketData(
             CThostFtdcSpecificInstrumentField *pSpecificInstrument,
             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << " Response | [OnRspUnSubMarketData] : " << ((pRspInfo->ErrorID == 0) ? "success" : "fail") << "; DetailInfo : " << pRspInfo->ErrorMsg << endl;
  //if(bIsLast)  SetEvent(g_hEvent);
	SetEvent(g_hEvent);
}

void CtpMdSpi::OnRtnDepthMarketData(
             CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	cerr << "................\n"
	<< "\n TradingDay: " << pDepthMarketData->TradingDay
	<< "\n InstrumentID: " << pDepthMarketData->InstrumentID
	<< "\n ExchangeID: " << pDepthMarketData->ExchangeID
	<< "\n ExchangeInstID: " << pDepthMarketData->ExchangeInstID
	<< "\n LastPrice: " << pDepthMarketData->LastPrice
	<< "\n PreSettlementPrice: " << pDepthMarketData->PreSettlementPrice
	<< "\n PreClosePrice: " << pDepthMarketData->PreClosePrice
	<< "\n PreOpenInterest: " << pDepthMarketData->PreOpenInterest
	<< "\n OpenPrice: " << pDepthMarketData->OpenPrice
	<< "\n HighestPrice: " << pDepthMarketData->HighestPrice
	<< "\n LowestPrice: " << pDepthMarketData->LowestPrice
	<< "\n Volume: " << pDepthMarketData->Volume
	<< "\n Turnover: " << pDepthMarketData->Turnover
	<< "\n OpenInterest: " << pDepthMarketData->OpenInterest
	<< "\n ClosePrice: " << pDepthMarketData->ClosePrice
	<< "\n SettlementPrice: " << pDepthMarketData->SettlementPrice
	<< "\n UpperLimitPrice: " << pDepthMarketData->UpperLimitPrice
	<< "\n LowerLimitPrice:" << pDepthMarketData->LowerLimitPrice
	<< "\n PreDelta: " << pDepthMarketData->PreDelta
	<< "\n CurrDelta: " << pDepthMarketData->CurrDelta
	<< "\n UpdateTime: " << pDepthMarketData->UpdateTime
	<< "\n UpdateMillisec: " << pDepthMarketData->UpdateMillisec
	<< "\n BidPrice1: " << pDepthMarketData->BidPrice1
	<< "\n BidVolume1: " << pDepthMarketData->BidVolume1
	<< "\n AskVolume5: " << pDepthMarketData->AskVolume5
	<< "\n AveragePrice: " << pDepthMarketData->AveragePrice
	<< "\n ActionDay: " << pDepthMarketData->ActionDay << std::endl;

	//Important Node:
	//1) must create the local variable "pool" here, otherwise it will not call destruction fucntion when exit(0)
	//2) can't define a local RealTimeDataProcessor variable here, otherwise it will plus the ref-count, so that it will not call destruction fucntion when exit(0)
	auto pool = RealTimeDataProcessorPool::getInstance();
	pool->GenRealTimeDataProcessor(pDepthMarketData->InstrumentID)->AppendRealTimeData(CThostFtdcDepthMDFieldWrapper(pDepthMarketData));

	TryTerminate(pDepthMarketData->UpdateTime);
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
    cerr<<" Response | "<<pRspInfo->ErrorMsg<<endl;
  }
  return ret;
}