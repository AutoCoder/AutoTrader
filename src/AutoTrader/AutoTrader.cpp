// AutoTrader.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <future>
#include <condition_variable>

#include "spdlog/spdlog.h"
#include "ThreadSafeQueue.h"
#include "unittest.h"

#include "DBWrapper.h"
#include "Config.h"
#include "tradespi.h"
#include "mdspi.h"
#include "RealTimeDataProcessorPool.h"
#include "Order.h"
#include "TickWrapper.h"
#include "CommonUtils.h"
#include "AP_Mgr.h"
#include "socket_server.h"
#include "remote_user_action.h"
#include "ActionProcessor.h"

#include "RealTimeDataProcessor.h"
#include "LoadStrategies.h"
#include "AccountMgr.h"
#include "crossplatform.h"
#include <signal.h>

int requestId = 0;

std::mutex mtx;
std::condition_variable cv_md;
std::condition_variable cv_trade;
std::condition_variable cv_quit;
std::atomic<bool> g_quit(false);
std::atomic<bool> g_reply(false);
threadsafe_queue<Order> order_queue;

//std::mutex g_OrderRunMtx;

//void ExcuteOrderQueue(CtpTradeSpi* pUserSpi){
//	SYNC_PRINT << "Start to trade";
//	SYNC_PRINT << "Start to loop order queue";
//
//	while (true){
//		//g_OrderRunMtx.lock();// synchronize the order execute process
//		Order ord;
//		if (!order_queue.empty() && order_queue.try_pop(ord)){ // if pop success
//
//			//撤销该order同一合约的pending合约
//			pUserSpi->CancelOrder(ord.GetTriggerTick(), 6, ord.GetInstrumentId());
//
//			SYNC_PRINT << "Excute Order regarding instrumentID:" << ord.GetInstrumentId();
//			pUserSpi->ReqOrderInsert(ord);
//		}
//
//		if (g_quit/* && order_queue.empty()*/) //todo : close position
//			break;
//
//		sleep(500);
//	}
//
//	SYNC_PRINT << "End to loop order queue";
//}

#ifdef MUSTIMPL
void ReplayTickDataFromDB(const std::string& instrumentID, const std::string& strategyName, const std::string& posCtlName, const std::string& mark)
{
	SYNC_PRINT << "Reply " << instrumentID << " data from db";
	DBWrapper dbwrapper;
	g_reply = true;
	auto pool = RealTimeDataProcessorPool::getInstance();
	//TODO: Get 2000 dataItem from db per 

	//todo: get strategy pointer by strategyName, get IPositionCtl* by posCtlName
	std::unique_ptr<MACrossStratgy> p = std::make_unique<MACrossStratgy>(3, 5, (IPositionControl*)NULL);
	auto processor = std::make_shared<RealTimeDataProcessor>(p.get(), instrumentID, (Account*)NULL);

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
			pool->AppendRealTimeData(dataItem);
		}
	}
	SYNC_PRINT << "Reply " << instrumentID << " finished.";
	//Store to db
	pool->StoreStrategySequenceToDB(instrumentID, mark);
}
#endif

Transmission::socket_server server(2007);

void schedule_stop(int sig) {
	g_quit = true;
	server.stop();
};
/*
Usage: 
   AutoTrade.exe
   AutoTrade.exe replay rb1510 table_mark
*/
int main(int argc, const char* argv[]){
	signal(SIGINT, schedule_stop);
	StrategyPluginsLoader loader; //must be top
	auto console = spdlog::stdout_logger_mt("console");
	auto file_logger = spdlog::rotating_logger_mt("file_logger", "logs/logfile", 1048576 * 5, 3);
	file_logger->info() << "----------------------------------";
	file_logger->info() << "Lanuch AutoTrader...";
	//Test
	//RunUnitTest();

	if (argc == 6 && strcmp(argv[1], "replay") == 0){
#ifdef MUSTIMPL
		ReplayTickDataFromDB(argv[2], argv[3], argv[4], argv[5]);
#endif
	}
	else{
		auto pool = RealTimeDataProcessorPool::getInstance();
		
		//******setup Account Pool**********
		auto config = Config::Instance();

		//******Init md thread*******
		CThostFtdcMdApi* pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
		CtpMdSpi* pMdUserSpi = new CtpMdSpi(pMdUserApi, Account::Manager::Instance().Instruments(), config->DefaultCtpBrokerID(), config->DefaultCtpUserID(), config->DefaultCtpPassword());
		pMdUserApi->RegisterSpi(pMdUserSpi);
		pMdUserApi->RegisterFront(const_cast<char*>(Config::Instance()->CtpMdFront().c_str()));
		pMdUserApi->Init();
		//std::thread actionInvoker(ProcessActionQueue);

		std::future<bool> actionInvoker_result = std::async(std::launch::async, ProcessActionQueue);
		std::future<bool> server_result = std::async(std::launch::async, []() ->bool {
			server.run();
			return true;
		});
		
		if (server_result.get() == true){
			SYNC_LOG << "1) Shutdown Socket Server...Success";
		}

		//if (actionInvoker_result.get() == true){
		//	SYNC_LOG << "2) Shutdown Action Queue...Success";
		//}

		//write to db
		//pool->FreeProcessors();
	}
	return 0;
}