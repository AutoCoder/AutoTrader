// AutoTrader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "printer.h"
#include "DBWrapper.h"

void test_md(void){
	
	CThostFtdcMdApi* pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
	CtpMdSpi* pUserSpi = new CtpMdSpi(pUserApi); 
	pUserApi->RegisterSpi(pUserSpi);			
	pUserApi->RegisterFront(mdFront);		 

	pUserApi->Init();     
	ShowMdCommand(pUserSpi, true);
	pUserApi->Join();      
}

void test_order(void)
{
	CThostFtdcTraderApi* pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	CtpTraderSpi* pUserSpi = new CtpTraderSpi(pUserApi);
	pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);			
	pUserApi->SubscribePublicTopic(THOST_TERT_RESTART);					
	pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);			 
	pUserApi->RegisterFront(tradeFront);							

	pUserApi->Init();
	WaitForSingleObject(g_hEvent, INFINITE);
	ResetEvent(g_hEvent);
	ShowTraderCommand(pUserSpi, true);
	pUserApi->Join();
	//pUserApi->Release();
}

void MonitorInstruments(CtpMdSpi* p, char* instrumentIds)
{
	//wait for frontconnected
	WaitForSingleObject(g_hEvent, INFINITE);
	ResetEvent(g_hEvent);

	//login
	cerr << "Start Moniter Instrument: " << instrumentIds;
	cerr << "\n BrokerID > " << "0292\n";
	cerr << " UserID > " << "00127\n";
	cerr << " Password > " << "asdfgh\n";
	p->ReqUserLogin("0292", "00127", "asdfgh");

	//wait for login response received
	WaitForSingleObject(g_hEvent, INFINITE);
	ResetEvent(g_hEvent);

	p->SubscribeMarketData(instrumentIds);
	WaitForSingleObject(g_hEvent, INFINITE); // wait for Subscribe success
	ResetEvent(g_hEvent);
}

void StartInstrumentMonitor(char* instrumentIds){
	CThostFtdcMdApi* pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
	CtpMdSpi* pUserSpi = new CtpMdSpi(pUserApi);
	pUserApi->RegisterSpi(pUserSpi);
	pUserApi->RegisterFront(mdFront);

	pUserApi->Init();
	MonitorInstruments(pUserSpi, instrumentIds);
	pUserApi->Join();
}

int main(int argc, const char* argv[]){
	g_hEvent = CreateEvent(NULL, true, false, NULL);

	//todo : read instruments from file
	char instruments[] = "rb1510,rb1511";
	StartInstrumentMonitor(instruments);

	//DBWrapper::GetDBWrapper().ExecuteNoResult("")
	//int ret = DBUtils::CreateTickTableIfNotExists("qihuo", "rb1511");

	if (argc < 2)  cerr << "miss arguments." << endl;
	else if (strcmp(argv[1], "--md") == 0)    test_md();
	else if (strcmp(argv[1], "--order") == 0) test_order();

	std::cerr << "quit ... " << std::endl;
	return 0;
}