// AutoTrader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "printer.h"
#include "DBWrapper.h"
#include "config.h"
#include "AccountManger.h"
#include <thread>
#include "RealTimeDataProcessorPool.h"
#include <mutex>
#include <condition_variable>
#include "Order.h"
#include "OrderQueue.h"
#include <atomic>
#include "spdlog/spdlog.h"

int requestId = 0;
HANDLE g_hEvent;
HANDLE g_tradehEvent;

std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> g_quit = false;
threadsafe_queue<Order> order_queue;

void MonitorInstruments(CtpMdSpi* p, char* instrumentIds)
{
	//wait for frontconnected
	WaitForSingleObject(g_hEvent, INFINITE);
	ResetEvent(g_hEvent);

	//login
	spdlog::get("console")->info() << "Start Moniter Instrument: " << instrumentIds
		<< "BrokerID > " << Config::Instance()->CtpBrokerID()
		<< "UserID > " << Config::Instance()->CtpUserID()
		<< "Password > " << Config::Instance()->CtpPassword();
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

void StartMDLoginThread(CtpMdSpi* p){

	std::unique_lock <std::mutex> lck(mtx);
	while (!g_quit){
		cv.wait(lck, [&](){ return p->IsFrontConnected() && !p->IsLogin(); });

		spdlog::get("console")->info() << "Login...";
		spdlog::get("console")->info() << "\n> BrokerID > " << Config::Instance()->CtpBrokerID();
		spdlog::get("console")->info() << "\n> UserID > " << Config::Instance()->CtpUserID();
		spdlog::get("console")->info() << "\n> Password > " << Config::Instance()->CtpPassword();
		p->ReqUserLogin(const_cast<char*>(Config::Instance()->CtpBrokerID().c_str()) \
			, const_cast<char*>(Config::Instance()->CtpUserID().c_str())\
			, const_cast<char*>(Config::Instance()->CtpPassword().c_str()));

		cv.wait(lck, [&](){ return p->IsLogin() && !p->IsSubscribed();});
		p->SubscribeMarketData(const_cast<char*>(Config::Instance()->CtpInstrumentIDs().c_str()));
	}
}

void StartTradeThread(CThostFtdcTraderApi* pUserApi, AccountMangerSpi* pUserSpi)
{
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

	pUserSpi->ReqSettlementInfoConfirm();
	WaitForSingleObject(g_tradehEvent, INFINITE);
	ResetEvent(g_tradehEvent);

	pUserSpi->ReqQryTradingAccount();
	WaitForSingleObject(g_tradehEvent, INFINITE);
	ResetEvent(g_tradehEvent);

	//pUserSpi->ExcuteOrderQueue();
}

void ExcuteOrderQueue(AccountMangerSpi* pUserSpi){
	spdlog::get("console")->info() << "Start to trade";
	spdlog::get("console")->info() << "Start to loop order queue";
	while (true){
		Order ord;
		if (!order_queue.empty() && order_queue.try_pop(ord)){ // if pop success
			spdlog::get("console")->info() << "Excute Order regarding instrumentID:" << ord.GetInstrumentId();
			//Todo: according ord to insert order
		}

		if (g_quit && order_queue.empty())
			break;

		//query accout to refresh the cashed the investor position
		// todo : sleep 500ms
		Sleep(500);
	}

	spdlog::get("console")->info() << "End to loop order queue";
}

int main(int argc, const char* argv[]){
	
	auto console = spdlog::stdout_logger_mt("console");

	g_hEvent = CreateEvent(NULL, true, false, NULL);
	g_tradehEvent = CreateEvent(NULL, true, false, NULL);

	//[Begin]******start md thread*******
	CThostFtdcMdApi* pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
	CtpMdSpi* pUserSpi = new CtpMdSpi(pMdUserApi);
	pMdUserApi->RegisterSpi(pUserSpi);
	pMdUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpMdFront().c_str()));
	pMdUserApi->Init();

	//Create a thread, Once FrontDisconnect ,try to reconnect and subscribe MD again if needed.
	std::thread mdthread(StartMDLoginThread, pUserSpi);
	//[End]******start md thread******

	//[Begin]*******start trade thread********
	CThostFtdcTraderApi* pTradeUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	AccountMangerSpi* pTradeUserSpi = new AccountMangerSpi(pTradeUserApi, \
		Config::Instance()->CtpBrokerID().c_str(), \
		Config::Instance()->CtpUserID().c_str(), \
		Config::Instance()->CtpPassword().c_str());
	StartTradeThread(pTradeUserApi, pTradeUserSpi);

	//[Excute Order Thread] Excute the Order in Queue one by one looply.
	std::thread tradeThread(ExcuteOrderQueue, pTradeUserSpi);
	//[End]*******start trade thread*******

	//[Main Thread]Release the resource and pointer.
#if 0
	while (true){
		if (g_quit){
			console->info() << "Start to release resource...";
			if (pMdUserApi)
			{
				pMdUserApi->RegisterSpi(NULL);
				pMdUserApi->Release();
				pMdUserApi = NULL;
			}
			if (pUserSpi)
			{
				delete pUserSpi;
				pUserSpi = NULL;
			}
			if (pTradeUserApi)
			{
				pTradeUserApi->RegisterSpi(NULL);
				pTradeUserApi->Release();
				pTradeUserApi = NULL;
			}
			if (pTradeUserSpi)
			{
				delete pTradeUserSpi;
				pTradeUserSpi = NULL;
			}
			console->info() << "Quit ... ";
			exit(0);
		}
		Sleep(1000);
	}
#endif

	return 0;
}
