// AutoTrader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "crossplatform.h"
#include "printer.h"
#include "DBWrapper.h"
#include "config.h"
#include "tradespi.h"
#include <thread>
#include "RealTimeDataProcessorPool.h"
#include <mutex>
#include <condition_variable>
#include "Order.h"
#include "OrderQueue.h"
#include <atomic>
#include "spdlog/spdlog.h"
#include "TickWrapper.h"
#include "unittest.h"
#include "CommonUtils.h"
#include "IAccount.h"
#include "BaseAccountMgr.h"

int requestId = 0;

std::mutex mtx;
std::condition_variable cv_md;
std::condition_variable cv_trade;
std::atomic<bool> g_quit = false;
std::atomic<bool> g_reply = false;
threadsafe_queue<Order> order_queue;

void MdManageThread(CtpMdSpi* pMdUserSpi){

	std::unique_lock <std::mutex> lck(mtx);
	while (!g_quit){
		cv_md.wait(lck);
		if (pMdUserSpi->IsFrontConnected() && !pMdUserSpi->IsLogin()){
			pMdUserSpi->ReqUserLogin(const_cast<char*>(Config::Instance()->CtpBrokerID().c_str()) \
				, const_cast<char*>(Config::Instance()->CtpUserID().c_str())\
				, const_cast<char*>(Config::Instance()->CtpPassword().c_str()));
		}
		else if (pMdUserSpi->IsLogin() && !pMdUserSpi->IsSubscribed()){
			pMdUserSpi->SubscribeMarketData(const_cast<char*>(Config::Instance()->CtpInstrumentIDs().c_str()));
		}
	}
}

void TradeManageThread(CtpTradeSpi* pTradeUserSpi){
	std::unique_lock <std::mutex> lck(mtx);
	while (!g_quit){
		cv_trade.wait(lck);

		if (pTradeUserSpi->IsFrontConnected() && !pTradeUserSpi->IsLogin()){
			pTradeUserSpi->ReqUserLogin();
		}
		else if (pTradeUserSpi->IsLogin() && !pTradeUserSpi->IsConfirmedSettlementInfo()){
			pTradeUserSpi->ReqSettlementInfoConfirm();
		}
		else if (pTradeUserSpi->IsConfirmedSettlementInfo() && !pTradeUserSpi->IsAccoutRefreshed()){
			pTradeUserSpi->ReqQryTradingAccount();
			pTradeUserSpi->ReqQryInvestorPosition();
		}
	}
}

void ExcuteOrderQueue(CtpTradeSpi* pUserSpi){
	spdlog::get("console")->info() << "Start to trade";
	spdlog::get("console")->info() << "Start to loop order queue";

	while (true){
		Order ord;
		if (!order_queue.empty() && order_queue.try_pop(ord)){ // if pop success
			spdlog::get("console")->info() << "Excute Order regarding instrumentID:" << ord.GetInstrumentId();
			pUserSpi->ReqOrderInsert(ord);
		}

		if (g_quit && order_queue.empty())
			break;

		sleep(500);
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

	if (countResult.empty()){
		spdlog::get("console")->info() << "Get 0 md record from db, please check db connection configuration";
		return;
	}

	long long totalCount = CommonUtils::StringtoInt(countResult[0][0]);

	int pagesize = 1000; 
	for (int i = 0; i < (totalCount / pagesize + 1); i++){
		const char * sqlselect = "select * from %s.%s order by id limit %ld,%d;";

		char sqlbuf[512];
		sprintf_s(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), instrumentID.c_str(), i*pagesize, pagesize);

		std::map<int, std::vector<std::string>> map_results;
		dbwrapper.Query(sqlbuf, map_results);

		for (auto item : map_results){
			auto dataItem = TickWrapper::RecoverFromDB(item.second);
			pool->GenRealTimeDataProcessor(instrumentID)->AppendRealTimeData(dataItem);
		}
	}
	spdlog::get("console")->info() << "Reply " << instrumentID << " finished.";
	//Store to db
	pool->GenRealTimeDataProcessor(instrumentID)->StoreStrategySequenceToDB(mark);
}

/*
Usage: 
   AutoTrade.exe
   AutoTrade.exe replay rb1510 table_mark
*/
int main(int argc, const char* argv[]){
	
	auto console = spdlog::stdout_logger_mt("console");

	//Test
	//RunUnitTest();

	if (argc == 4 && strcmp(argv[1], "replay") == 0){
		ReplayTickDataFromDB(argv[2], argv[3]);
	}
	else{
		auto pool = RealTimeDataProcessorPool::getInstance();
		
		//******Init md thread*******
		CThostFtdcMdApi* pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
		CtpMdSpi* pMdUserSpi = new CtpMdSpi(pMdUserApi);
		pMdUserApi->RegisterSpi(pMdUserSpi);
		pMdUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpMdFront().c_str()));

		//*******Init trade thread********
		CThostFtdcTraderApi* pTradeUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
		CtpTradeSpi* pTradeUserSpi = new CtpTradeSpi(pTradeUserApi, \
			Config::Instance()->CtpBrokerID().c_str(), \
			Config::Instance()->CtpUserID().c_str(), \
			Config::Instance()->CtpPassword().c_str(), \
			Config::Instance()->ProductName().c_str());
		pTradeUserApi->RegisterSpi((CThostFtdcTraderSpi*)pTradeUserSpi);
		pTradeUserApi->SubscribePublicTopic(THOST_TERT_RESTART);
		pTradeUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);
		pTradeUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpTradeFront().c_str()));

		//*******pool's process's stratgy's accountMgr listen to account updation received from pTradeUserSpi.
		pool->ListenToTradeSpi(pTradeUserSpi);

		//Create a thread, Once FrontDisconnect ,try to reconnect and subscribeMD again if needed.
		std::thread mdManagethread(MdManageThread, pMdUserSpi);
		std::thread tradeManagethread(TradeManageThread, pTradeUserSpi);
		//[Excute Order Thread] Excute the Order in Queue one by one looply.
		std::thread tradeThread(ExcuteOrderQueue, pTradeUserSpi);

		//******start md thread******
		pMdUserApi->Init();
		//******start trade thread******
		pTradeUserApi->Init();

		mdManagethread.join();
		tradeManagethread.join();
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
