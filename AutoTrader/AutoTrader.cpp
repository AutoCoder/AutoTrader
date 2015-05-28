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
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "unittest.h"
#include "CommonUtils.h"

int requestId = 0;

std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> g_quit = false;
std::atomic<bool> g_reply = false;
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

		if (pTradeUserSpi->IsFrontConnected() && !pTradeUserSpi->IsLogin()){
			pTradeUserSpi->ReqUserLogin();
		}
		else if (pTradeUserSpi->IsLogin() && !pTradeUserSpi->IsConfirmedSettlementInfo()){
			pTradeUserSpi->ReqSettlementInfoConfirm();
		}
		else if (pTradeUserSpi->IsConfirmedSettlementInfo()){
			pTradeUserSpi->ReqQryTradingAccount();
			pTradeUserSpi->ReqQryInvestorPosition("rb1510");
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

void ReplayTickDataFromDB(const std::string& instrumentID, const std::string& mark)
{
	spdlog::get("console")->info() << "Reply " << instrumentID << " data from db";
	DBWrapper dbwrapper;
	g_reply = true;
	auto pool = RealTimeDataProcessorPool::getInstance();
	//TODO: Get 2000 dataItem from db per 

	//Get the total count of table
	char countquerybuf[512];
	const char* countquery = "select count(*) from %s.%s order by id;";
	sprintf_s(countquerybuf, countquery, Config::Instance()->DBName().c_str(), instrumentID.c_str());
	std::map<int, std::vector<std::string>> countResult;
	dbwrapper.Query(countquerybuf, countResult);

	long long totalCount = CommonUtils::StringtoInt(countResult[0][0]);

	//PageSize = 100;
	int pagesize = 100; 
	for (int i = 0; i < (totalCount / pagesize + 1); i++){
		const char * sqlselect = "select * from %s.%s order by id limit %ld,%d;";

		char sqlbuf[512];
		sprintf_s(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), instrumentID.c_str(), i*pagesize, pagesize);

		std::map<int, std::vector<std::string>> map_results;
		dbwrapper.Query(sqlbuf, map_results);

		for (auto item : map_results){
			auto dataItem = CThostFtdcDepthMDFieldWrapper::RecoverFromDB(item.second);
			pool->GenRealTimeDataProcessor(instrumentID)->AppendRealTimeData(dataItem);
		}
	}

	//Store to db
	pool->GenRealTimeDataProcessor(instrumentID)->StoreStrategySequenceToDB(mark);
}

/*
Usage: 
   AutoTrade.exe
   AutoTrade.exe replay rb1510 table_mark
*/
int main(int argc, const char* argv[]){
	
	//Test
	//RunUnitTest();

	auto console = spdlog::stdout_logger_mt("console");

	if (argc == 4 && strcmp(argv[1], "replay") == 0){
		ReplayTickDataFromDB(argv[2], argv[3]);
	}
	else{
		auto pool = RealTimeDataProcessorPool::getInstance();

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
			Config::Instance()->CtpPassword().c_str(), \
			Config::Instance()->ProductName().c_str());
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

		//write to db
		pool->FreeProcessors();
	}

	return 0;
}
