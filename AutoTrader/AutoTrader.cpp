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

void StartLoginThread(CtpMdSpi* pMdUserSpi, AccountMangerSpi* pTradeUserSpi){

	std::unique_lock <std::mutex> lck(mtx);
	while (!g_quit){
		cv.wait(lck);
		if (pMdUserSpi->IsFrontConnected() && !pMdUserSpi->IsLogin()){
			spdlog::get("console")->info() << "Login...";
			spdlog::get("console")->info() << "\n> BrokerID > " << Config::Instance()->CtpBrokerID();
			spdlog::get("console")->info() << "\n> UserID > " << Config::Instance()->CtpUserID();
			spdlog::get("console")->info() << "\n> Password > " << Config::Instance()->CtpPassword();
			pMdUserSpi->ReqUserLogin(const_cast<char*>(Config::Instance()->CtpBrokerID().c_str()) \
				, const_cast<char*>(Config::Instance()->CtpUserID().c_str())\
				, const_cast<char*>(Config::Instance()->CtpPassword().c_str()));
		}
		else if (pMdUserSpi->IsLogin() && !pMdUserSpi->IsSubscribed()){
			pMdUserSpi->SubscribeMarketData(const_cast<char*>(Config::Instance()->CtpInstrumentIDs().c_str()));
		}
		else if (pTradeUserSpi->IsFrontConnected() && !pTradeUserSpi->IsLogin()){
			pTradeUserSpi->ReqUserLogin();
		}
		else if (pTradeUserSpi->IsLogin() && !pTradeUserSpi->IsConfirmedSettlementInfo()){
			pTradeUserSpi->ReqSettlementInfoConfirm();
		}
	}
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
	CtpMdSpi* pMdUserSpi = new CtpMdSpi(pMdUserApi);
	pMdUserApi->RegisterSpi(pMdUserSpi);
	pMdUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpMdFront().c_str()));
	pMdUserApi->Init();
	//[End]******start md thread******

	//[Begin]*******start trade thread********
	CThostFtdcTraderApi* pTradeUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	AccountMangerSpi* pTradeUserSpi = new AccountMangerSpi(pTradeUserApi, \
		Config::Instance()->CtpBrokerID().c_str(), \
		Config::Instance()->CtpUserID().c_str(), \
		Config::Instance()->CtpPassword().c_str());
	pTradeUserApi->RegisterSpi((CThostFtdcTraderSpi*)pTradeUserSpi);
	pTradeUserApi->SubscribePublicTopic(THOST_TERT_RESTART);
	pTradeUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);
	pTradeUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpTradeFront().c_str()));
	pTradeUserApi->Init();
	//[End]******start trade thread******

	//Create a thread, Once FrontDisconnect ,try to reconnect and subscribeMD again if needed.
	std::thread loginthread(StartLoginThread, pMdUserSpi, pTradeUserSpi);
	//[Excute Order Thread] Excute the Order in Queue one by one looply.
	std::thread tradeThread(ExcuteOrderQueue, pTradeUserSpi);

	loginthread.join();
	tradeThread.join();

#if 1
	//[Main Thread]Release the resource and pointer.
	console->info() << "Start to release resource...";
	if (pMdUserApi)
	{
		pMdUserApi->RegisterSpi(NULL);
		pMdUserApi->Release();
		pMdUserApi = NULL;
	}
	if (pMdUserSpi)
	{
		delete pMdUserSpi;
		pMdUserSpi = NULL;
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
#endif

	//todo: write to db
	
	return 0;
}
