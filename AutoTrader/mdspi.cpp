#include "stdafx.h"
#include "mdspi.h"
#include <iostream>
#include <vector>
#include "windows.h"
#include "Order.h"
#include "OrderQueue.h"
#include "Strategy.h"


using namespace std;
#pragma warning(disable : 4996)

static threadsafe_queue<Order> order_queue;

extern int requestId;  
extern HANDLE g_hEvent;


CtpMdSpi::CtpMdSpi(CThostFtdcMdApi* api) 
	:pUserApi(api)
{
	m_strategies.push_back(k3UpThroughK5());
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
  if(bIsLast) SetEvent(g_hEvent);
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
}

void CtpMdSpi::OnRspUnSubMarketData(
             CThostFtdcSpecificInstrumentField *pSpecificInstrument,
             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	cerr << " Response | [OnRspUnSubMarketData] : " << ((pRspInfo->ErrorID == 0) ? "success" : "fail") << "; DetailInfo : " << pRspInfo->ErrorMsg << endl;
  //if(bIsLast)  SetEvent(g_hEvent);
}

void CtpMdSpi::OnRtnDepthMarketData(
             CThostFtdcDepthMarketDataField *pDepthMarketData)
{
  cerr<<" md | Instrument:"<<pDepthMarketData->InstrumentID
	<< "\n Timestamp:" << pDepthMarketData->TradingDay
    <<"\n price:"<<pDepthMarketData->LastPrice
    <<"\n Highest price:" << pDepthMarketData->HighestPrice
    <<"\n Lowest price:" << pDepthMarketData->LowestPrice
    <<"\n ask price1:" << pDepthMarketData->AskPrice1 
    <<"\n ask volume1:" << pDepthMarketData->AskVolume1 
    <<"\n Bid price1:" << pDepthMarketData->BidPrice1
    <<"\n Bid volume1:" << pDepthMarketData->BidVolume1
    <<"\n OpenInterest:"<< pDepthMarketData->OpenInterest 
	<<"\n Average Price:" << pDepthMarketData->AveragePrice << endl;
  AppendRealTimeData(CThostFtdcDepthMDFieldWrapper(pDepthMarketData));
  SetEvent(g_hEvent);
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
    cerr<<" Response | "<<pRspInfo->ErrorMsg<<endl;
  }
  return ret;
}

bool CtpMdSpi::AppendRealTimeData(CThostFtdcDepthMDFieldWrapper& info){
	//(in)front-------------back(out)
	if (m_DataSeq.size() >= QueueSize){
		const CThostFtdcDepthMDFieldWrapper& firstDataBlock = m_DataSeq.back();
		firstDataBlock.serializeToDB();
		m_DataSeq.pop_back();
	}

	//loop the Strategies
	for (auto&& item : m_strategies){
		if (item.TryInvoke(m_DataSeq, info)){
			order_queue.push(item.generateOrder());
		}
	}
	return false;
}