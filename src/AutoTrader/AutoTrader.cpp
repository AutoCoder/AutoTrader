// AutoTrader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DBWrapper.h"
#include "config.h"
#include "tradespi.h"
#include "mdspi.h"
#include "RealTimeDataProcessorPool.h"
#include "Order.h"
#include "OrderQueue.h"
#include "spdlog/spdlog.h"
#include "TickWrapper.h"
#include "unittest.h"
#include "CommonUtils.h"
#include "IPositionControl.h"
#include "AP_Mgr.h"

#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

int requestId = 0;

std::mutex mtx;
std::condition_variable cv_md;
std::condition_variable cv_trade;
std::atomic<bool> g_quit(false);
std::atomic<bool> g_reply(false);
threadsafe_queue<Order> order_queue;

//std::mutex g_OrderRunMtx;

void ExcuteOrderQueue(CtpTradeSpi* pUserSpi){
	SYNC_PRINT << "Start to trade";
	SYNC_PRINT << "Start to loop order queue";

	while (true){
		//g_OrderRunMtx.lock();// synchronize the order execute process
		Order ord;
		if (!order_queue.empty() && order_queue.try_pop(ord)){ // if pop success

			//撤销该order同一合约的pending合约
			pUserSpi->CancelOrder(ord.GetTriggerTick(), 6, ord.GetInstrumentId());

			SYNC_PRINT << "Excute Order regarding instrumentID:" << ord.GetInstrumentId();
			pUserSpi->ReqOrderInsert(ord);
		}

		if (g_quit/* && order_queue.empty()*/) //todo : close position
			break;

		sleep(500);
	}

	SYNC_PRINT << "End to loop order queue";
}

void ReplayTickDataFromDB(const std::string& instrumentID, const std::string& mark)
{
	SYNC_PRINT << "Reply " << instrumentID << " data from db";
	DBWrapper dbwrapper;
	g_reply = true;
	auto pool = RealTimeDataProcessorPool::getInstance();
	//TODO: Get 2000 dataItem from db per 

	//Get the total count of table
	char countquerybuf[512];
	const char* countquery = "select count(*) from %s.%s order by id;";
	SPRINTF(countquerybuf, countquery, Config::Instance()->DBName().c_str(), instrumentID.c_str());
	std::map<int, std::vector<std::string>> countResult;
	dbwrapper.Query(countquerybuf, countResult);

	if (countResult.empty()){
		SYNC_PRINT << "Get 0 md record from db, please check db connection configuration";
		return;
	}

	long long totalCount = CommonUtils::StringtoInt(countResult[0][0]);

	int pagesize = 1000; 
	for (int i = 0; i < (totalCount / pagesize + 1); i++){
		const char * sqlselect = "select * from %s.%s order by id limit %ld,%d;";

		char sqlbuf[512];
		SPRINTF(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), instrumentID.c_str(), i*pagesize, pagesize);

		std::map<int, std::vector<std::string>> map_results;
		dbwrapper.Query(sqlbuf, map_results);

		for (auto item : map_results){
			auto dataItem = TickWrapper::RecoverFromDB(item.second);
			pool->GenRealTimeDataProcessor(instrumentID)->AppendRealTimeData(dataItem);
		}
	}
	SYNC_PRINT << "Reply " << instrumentID << " finished.";
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

		/*depreted:
		//Create a thread, Once FrontDisconnect ,try to reconnect and subscribeMD again if needed.
		//std::thread mdManagethread(MdManageThread, pMdUserSpi);
		//std::thread tradeManagethread(TradeManageThread, pTradeUserSpi);
		*/

		//******start trade thread******
		pTradeUserApi->Init();

		//******start md thread******
		std::unique_lock <std::mutex> lck(mtx);
		cv_md.wait(lck);
		pMdUserApi->Init();

		//[Excute Order Thread] Excute the Order in Queue one by one looply.
		std::thread tradeThread(ExcuteOrderQueue, pTradeUserSpi);

		//mdManagethread.join();
		//tradeManagethread.join();
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
