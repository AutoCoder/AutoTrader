#include "stdafx.h"
#include "mdspi.h"
#include "TickWrapper.h"
#include "MdProcessorPool.h"
#include "spdlog/spdlog.h"
#include "CommonUtils.h"
#include "crossplatform.h"
#include "DBWrapper.h"

#include <iostream>
#include <condition_variable>
#include <atomic>
#include <csignal>

#ifdef WIN32
#pragma warning(disable : 4996)
#endif

//namespace {
//
//	void TryTerminate(const char * time){
//		if (/*strcmp(time, "10:55:00") == 0){ //*/CommonUtils::IsMarketingTime(time) == false){
//			raise(SIGINT);
//		}
//	}
//}


CtpMdSpi::CtpMdSpi(CThostFtdcMdApi* api, const std::vector<std::string>& instruments, const std::string& brokerId, const std::string& userID, const std::string& pw)
	: pUserApi(api)
	, m_requestId(0)
	, m_stateChangeHandler(this, instruments, brokerId, userID, pw)
	, m_dbptr(new DBWrapper)
{
	for (auto instru : instruments){
		m_TickMap[instru] = {};
		m_TickMap60[instru] = {};
		m_KDataMap[instru] = {};
	}
}

CtpMdSpi::~CtpMdSpi()
{
	SerializeToDb();
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
	int ret = pUserApi->ReqUserLogin(&req, ++m_requestId);
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
	SYNC_PRINT << "[Md] Response | [OnRspSubMarketData] : " << ((pRspInfo->ErrorID == 0) ? "success" : "fail") << "; to subscribe " << pSpecificInstrument->InstrumentID;
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
	//2) can't define a local MdProcessor variable here, otherwise it will plus the ref-count, so that it will not call destruction fucntion when exit(0)
	auto pool = MdProcessorPool::getInstance();
	TickWrapper tem(pDepthMarketData);
	
	pool->AppendTick(tem);
	UpdateCachedTickData(tem);
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  if (ret){
	  SYNC_PRINT << "[Md] Response | " << pRspInfo->ErrorMsg;
  }
  return ret;
}

std::vector<TickWrapper>& CtpMdSpi::GetTickVec(const std::string& instrument){
	assert(m_TickMap.find(instrument) != m_TickMap.end());
	return m_TickMap[instrument];
}

std::vector<TickWrapper>& CtpMdSpi::GetTickVec60(const std::string& instrument){
	assert(m_TickMap60.find(instrument) != m_TickMap60.end());
	return m_TickMap60[instrument];
}

std::vector<KData>& CtpMdSpi::GetKDataVec(const std::string& instrument){
	assert(m_KDataMap.find(instrument) != m_KDataMap.end());
	return m_KDataMap[instrument];
}

void CtpMdSpi::UpdateCachedTickData(const TickWrapper& tem){
	GetTickVec(tem.InstrumentId()).push_back(tem);

	if (m_TickMap60[tem.InstrumentId()].empty()){
		m_TickMap60[tem.InstrumentId()].push_back(tem);
	}
	else{
		if (CommonUtils::InSameMinute(tem.Time(), m_TickMap60[tem.InstrumentId()].front().Time())){
			m_TickMap60[tem.InstrumentId()].push_back(tem);
		}
		else{ // the comming tick data is in next minutes
			KData k1m(m_TickMap60[tem.InstrumentId()], 60);
			m_KDataMap[tem.InstrumentId()].push_back(k1m);
			m_TickMap60[tem.InstrumentId()].clear();
		}
	}
}

void CtpMdSpi::SerializeToDb(){

	SYNC_LOG << "Start to store MD Tick to db...";
	for (auto& kv : m_TickMap){
		for (auto tick : kv.second){
			tick.serializeToDB(*(m_dbptr.get()));
		}
	}
	SYNC_LOG << "Finish...";

	SYNC_LOG << "Start to store K-LINE to db...";
	for (auto& kv : m_KDataMap){
		for (auto kdata : kv.second){
			kdata.serializeToDB(*(m_dbptr.get()));
		}
	}
	SYNC_LOG << "Finish...";
}
