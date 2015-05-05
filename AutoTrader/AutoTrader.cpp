// AutoTrader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "printer.h"
#include "DBWrapper.h"
#include "config.h"
#include "AccountManger.h"
#include <thread>
#include "RealTimeDataProcessorPool.h"

int requestId = 0;
HANDLE g_hEvent;
HANDLE g_tradehEvent;

//void test_md(void){
//	
//	CThostFtdcMdApi* pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
//	CtpMdSpi* pUserSpi = new CtpMdSpi(pUserApi); 
//	pUserApi->RegisterSpi(pUserSpi);			
//	pUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpMdFront().c_str()));
//
//	pUserApi->Init();     
//	ShowMdCommand(pUserSpi, true);
//	pUserApi->Join();
//} 

//void test_order(void)
//{
//	CThostFtdcTraderApi* pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
//	CtpTraderSpi* pUserSpi = new CtpTraderSpi(pUserApi);
//	pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);			
//	pUserApi->SubscribePublicTopic(THOST_TERT_RESTART);					
//	pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);			 
//	pUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpTradeFront().c_str()));
//
//	pUserApi->Init();
//	WaitForSingleObject(g_hEvent, INFINITE);
//	ResetEvent(g_hEvent);
//	ShowTraderCommand(pUserSpi, true);
//	pUserApi->Join();
//	//pUserApi->Release();
//}

void MonitorInstruments(CtpMdSpi* p, char* instrumentIds)
{
	//wait for frontconnected
	WaitForSingleObject(g_hEvent, INFINITE);
	ResetEvent(g_hEvent);

	//login
	cerr << "Start Moniter Instrument: " << instrumentIds;
	cerr << "\n BrokerID > " << Config::Instance()->CtpBrokerID();
	cerr << "\n UserID > " << Config::Instance()->CtpUserID();
	cerr << "\n Password > " << Config::Instance()->CtpPassword() << std::endl;
	p->ReqUserLogin(const_cast<char*>(Config::Instance()->CtpBrokerID().c_str()) \
		, const_cast<char*>(Config::Instance()->CtpUserID().c_str())\
		, const_cast<char*>(Config::Instance()->CtpPassword().c_str()));

	//wait for login response received
	WaitForSingleObject(g_hEvent, INFINITE);
	ResetEvent(g_hEvent);

	p->SubscribeMarketData(instrumentIds);
	WaitForSingleObject(g_hEvent, INFINITE); // wait for Subscribe success
	ResetEvent(g_hEvent);
}

void StartMdThread(CThostFtdcMdApi* pUserApi, char* instrumentIds){
	//CThostFtdcMdApi* pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
	CtpMdSpi* pUserSpi = new CtpMdSpi(pUserApi);
	pUserApi->RegisterSpi(pUserSpi);
	pUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpMdFront().c_str()));

	pUserApi->Init();
	MonitorInstruments(pUserSpi, instrumentIds);
}

void StartTradeThread(CThostFtdcTraderApi* pUserApi)
{
	AccountMangerSpi* pUserSpi = new AccountMangerSpi(pUserApi, \
		Config::Instance()->CtpBrokerID().c_str(), \
		Config::Instance()->CtpUserID().c_str(), \
		Config::Instance()->CtpPassword().c_str());
	pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);
	pUserApi->SubscribePublicTopic(THOST_TERT_RESTART);
	pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);
	pUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpTradeFront().c_str()));

	pUserApi->Init();
	WaitForSingleObject(g_tradehEvent, INFINITE);
	ResetEvent(g_tradehEvent);

	//pUserApi->ReqAuthenticate();

	pUserSpi->ReqUserLogin();
	WaitForSingleObject(g_tradehEvent, INFINITE);
	ResetEvent(g_tradehEvent);


	//pUserSpi->

	pUserSpi->ReqSettlementInfoConfirm();
	WaitForSingleObject(g_tradehEvent, INFINITE);
	ResetEvent(g_tradehEvent);

	pUserSpi->ReqQryTradingAccount();
	WaitForSingleObject(g_tradehEvent, INFINITE);
	ResetEvent(g_tradehEvent);

	pUserSpi->ExcuteOrderQueue();
}

int main(int argc, const char* argv[]){
	RealTimeDataProcessorPool::getInstance();

	g_hEvent = CreateEvent(NULL, true, false, NULL);
	g_tradehEvent = CreateEvent(NULL, true, false, NULL);

	CThostFtdcMdApi* pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
	CThostFtdcTraderApi* pTradeUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	StartMdThread(pMdUserApi, const_cast<char*>(Config::Instance()->CtpInstrumentIDs().c_str()));

	StartTradeThread(pTradeUserApi);
	//std::thread tradeThread(mgr);
	exit(0);
	pMdUserApi->Join();
	pTradeUserApi->Join();
	
	//if (argc < 2)  cerr << "miss arguments." << endl;
	//else if (strcmp(argv[1], "--md") == 0)    test_md();
	//else if (strcmp(argv[1], "--order") == 0) test_order();

	std::cerr << "quit ... " << std::endl;
	return 0;
}