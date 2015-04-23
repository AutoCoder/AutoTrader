// AutoTrader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "printer.h"

void test_md(void){
	//³õÊ¼»¯UserApi
	CThostFtdcMdApi* pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
	CtpMdSpi* pUserSpi = new CtpMdSpi(pUserApi); //´´½¨»Øµ÷´¦ÀíÀà¶ÔÏóMdSpi
	pUserApi->RegisterSpi(pUserSpi);			// »Øµ÷¶ÔÏó×¢Èë½Ó¿ÚÀà
	pUserApi->RegisterFront(mdFront);		  // ×¢²áÐÐÇéÇ°ÖÃµØÖ·

	pUserApi->Init();      //½Ó¿ÚÏß³ÌÆô¶¯, ¿ªÊ¼¹¤×÷
	ShowMdCommand(pUserSpi, true);
	pUserApi->Join();      //µÈ´ý½Ó¿ÚÏß³ÌÍË³ö
	//pUserApi->Release(); //½Ó¿Ú¶ÔÏóÊÍ·Å
}

void test_order(void)
{
	//³õÊ¼»¯UserApi
	CThostFtdcTraderApi* pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	CtpTraderSpi* pUserSpi = new CtpTraderSpi(pUserApi);
	pUserApi->RegisterSpi((CThostFtdcTraderSpi*)pUserSpi);			// ×¢²áÊÂ¼þÀà
	pUserApi->SubscribePublicTopic(THOST_TERT_RESTART);					// ×¢²á¹«ÓÐÁ÷
	pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);			  // ×¢²áË½ÓÐÁ÷
	pUserApi->RegisterFront(tradeFront);							// ×¢²á½»Ò×Ç°ÖÃµØÖ·

	pUserApi->Init();
	WaitForSingleObject(g_hEvent, INFINITE);
	ResetEvent(g_hEvent);
	ShowTraderCommand(pUserSpi, true);
	pUserApi->Join();
	//pUserApi->Release();
}

void main(int argc, const char* argv[]){
	g_hEvent = CreateEvent(NULL, true, false, NULL);

	if (argc < 2)  cerr << "miss arguments." << endl;
	else if (strcmp(argv[1], "--md") == 0)    test_md();
	else if (strcmp(argv[1], "--order") == 0) test_order();

	std::cerr << "quit ... " << std::endl;
}