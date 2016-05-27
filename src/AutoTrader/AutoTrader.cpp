// AutoTrader.cpp : Defines the entry point for the console application.
//
//standard header
#include "stdafx.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <future>
#include <condition_variable>
#include <ctime>
#include <signal.h>

//headers in 3rdparty
#include "spdlog/spdlog.h"

//public headers in other module
//----Common Dll-----
#include "Config.h"
#include "DBWrapper.h"
#include "AccountMgr.h"
#include "CommonUtils.h"
#include "crossplatform.h"
//----StrategyFramework---
#include "Order.h"
#include "AP_Mgr.h"
#include "TickWrapper.h"

//private headers
//#include "unittest.h"
//#include <signal.h>
#include "mdspi.h"
#include "tradespi.h"
#include "LoadStrategies.h"
#include "ActionProcessor.h"
#include "ThreadSafeQueue.h"
#include "fifo_action_queue.h"
#include "MdProcessor.h"
#include "MdProcessorPool.h"
#include "socket_server.h"
#include "remote_user_action.h"
#include "LocalClientSession.h"

#pragma warning(disable:4996)

class RAII_MD{
public:
	RAII_MD()
		: pMdUserApi(nullptr)
		, pMdUserSpi(nullptr)
	{
		auto pool = MdProcessorPool::getInstance();
		auto config = Config::Instance();
		//Init md thread
		pMdUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
		pMdUserSpi = new CtpMdSpi(pMdUserApi, Account::Manager::Instance().Instruments(), config->DefaultCtpBrokerID(), config->DefaultCtpUserID(), config->DefaultCtpPassword());
		pMdUserApi->RegisterSpi(pMdUserSpi);
		pMdUserApi->RegisterFront(const_cast<char*>(config->CtpMdFront().c_str()));
		pMdUserApi->Init();
		pool->SetMdSpi(pMdUserSpi);
	}

	~RAII_MD(){
		if (pMdUserApi){
			pMdUserApi->RegisterSpi(NULL);
			pMdUserApi->Release();
			pMdUserApi = NULL;
		}

		if (pMdUserSpi) {
			delete pMdUserSpi;
			pMdUserSpi = NULL;
		}		
	}

private:
	CThostFtdcMdApi* 	pMdUserApi;
	CtpMdSpi* 			pMdUserSpi;
};

std::mutex q_mtx;
std::condition_variable q_cv;
std::atomic<bool> q_flag(false);

void ShutDown(int n){
	if (SIGINT == n || SIGTERM == n){
		q_flag.store(true);
		q_cv.notify_all();
	}

};

#ifdef MUSTIMPL
void ReplayTickDataFromDB(const std::string& instrumentID, const std::string& strategyName, const std::string& posCtlName, const std::string& mark)
{
	SYNC_PRINT << "Reply " << instrumentID << " data from db";
	DBWrapper dbwrapper;
	g_reply = true;
	auto pool = MdProcessorPool::getInstance();
	//TODO: Get 2000 dataItem from db per 

	//todo: get strategy pointer by strategyName, get IPositionCtl* by posCtlName
	std::unique_ptr<MACrossStratgy> p = std::make_unique<MACrossStratgy>(3, 5, (IPositionControl*)NULL);
	auto processor = std::make_shared<MdProcessor>(p.get(), instrumentID, (Account*)NULL, true);

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
			pool->AppendTick(dataItem);
		}
	}
	SYNC_PRINT << "Reply " << instrumentID << " finished.";
	//Store to db
	pool->StoreStrategySequenceToDB(instrumentID, mark);
}
#endif

void StartTradeLocally(const std::string& userID, const std::string& instrumentID, const std::string& strategyName, const std::string& db_mark){

	//Init md thread by RAII
	RAII_MD md;

	ActionQueueProcessor requestsProcessor(Transmission::GetRequestActionQueue()); 
	ActionQueueProcessor responseProcessor(Transmission::GetResponseActionQueue()); 

	std::future<bool> future_request_processor = std::async(std::launch::async, [&requestsProcessor]()->bool {
		requestsProcessor.Start();
		return true;
	});

	std::future<bool> future_response_processor = std::async(std::launch::async, [&responseProcessor]()->bool {
		responseProcessor.Start();
		return true;
	});

	LocalClientSession session(userID);
	if (session.Init_CTP()){
		ErrorCode err;
		session.StartTrade(instrumentID, strategyName, err);
	}


	//start schedule_stop function
	auto schedule_stop = std::async(std::launch::async, [&requestsProcessor, &responseProcessor](){
		std::unique_lock<std::mutex> lk(q_mtx);
		q_cv.wait(lk, []{return q_flag.load(); });
		requestsProcessor.Stop();
		responseProcessor.Stop();
	});

	//check if it's on trade available time periodically (15 min)
	auto check_quit = std::async(std::launch::async, [](){
		const int MilliSecondsPerQuarter = 15 * 60 * 1000;
		sleep(MilliSecondsPerQuarter); // 允许在非交易时间 运行15分钟
		while (q_flag.load() == false)
		{
			std::time_t result = std::time(nullptr);
			struct tm * now_local = std::localtime(&result);
			int second_elapse = now_local->tm_hour * 3600 + now_local->tm_min * 60 + now_local->tm_sec;
			if (CommonUtils::IsMarketingTime(second_elapse) == false)
			{
				ShutDown(SIGTERM);
			}
		}
	});

	if (future_response_processor.get() == true){
		SYNC_LOG << "1) Shutdown Response Action processor...Success";
	}

	if (future_request_processor.get() == true){
		SYNC_LOG << "1) Shutdown Request Action processor...Success";
	}		

	session.StopTrade();
}




void LaunchAutoTraderWithOfflineMd(const std::string& instrumentID, size_t spend = 500/*1 Tick duration(unit:ms)*/){
	ActionQueueProcessor requestsProcessor(Transmission::GetRequestActionQueue()); 
	ActionQueueProcessor responseProcessor(Transmission::GetResponseActionQueue()); 

	std::future<bool> future_request_processor = std::async(std::launch::async, [&requestsProcessor]()->bool {
		requestsProcessor.Start();
		return true;
	});

	std::future<bool> future_response_processor = std::async(std::launch::async, [&responseProcessor]()->bool {
		responseProcessor.Start();
		return true;
	});

	Transmission::socket_server server(2007);
	std::future<bool> future_server = std::async(std::launch::async, [&server]()->bool {
		server.run();
		return true;
	});

	
	//start schedule_stop function
	auto schedule_stop = std::async(std::launch::async, [&server, &requestsProcessor, &responseProcessor](){
		std::unique_lock<std::mutex> lk(q_mtx);
		q_cv.wait(lk, []{return q_flag.load(); });
		requestsProcessor.Stop();
		responseProcessor.Stop();
		server.stop();
	});

	signal(SIGINT, ShutDown);
	signal(SIGTERM, ShutDown);

	auto future_replay = std::async(std::launch::async, [&instrumentID, &spend](){
		//Get the total count of table
		DBWrapper dbwrapper;
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
		while (!q_flag.load()){
			for (int i = 0; i < (totalCount / pagesize + 1); i++){
				const char * sqlselect = "select * from %s.%s order by id limit %ld,%d;";

				char sqlbuf[512];
				SPRINTF(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), instrumentID.c_str(), i*pagesize, pagesize);

				std::map<int, std::vector<std::string>> map_results;
				dbwrapper.Query(sqlbuf, map_results);

				for (auto item : map_results){
					auto dataItem = TickWrapper::RecoverFromDB(item.second);
					MdProcessorPool::getInstance()->AppendTick(dataItem);
					sleep(spend);
				}
			}			
		}
		SYNC_PRINT << "Reply " << instrumentID << " Stop due to singal.";
	});

	if (future_response_processor.get() == true){
		SYNC_LOG << "1) Shutdown Response Action processor...Success";
	}

	if (future_request_processor.get() == true){
		SYNC_LOG << "2) Shutdown Request Action processor...Success";
	}		

	if (future_server.get() == true){
		SYNC_LOG << "3) Shutdown Socket Server...Success";
	}		
}

void LaunchAutoTrader(){
	//Init md thread by RAII
	RAII_MD md;

	ActionQueueProcessor requestsProcessor(Transmission::GetRequestActionQueue()); 
	ActionQueueProcessor responseProcessor(Transmission::GetResponseActionQueue()); 

	std::future<bool> future_request_processor = std::async(std::launch::async, [&requestsProcessor]()->bool {
		requestsProcessor.Start();
		return true;
	});

	std::future<bool> future_response_processor = std::async(std::launch::async, [&responseProcessor]()->bool {
		responseProcessor.Start();
		return true;
	});

	Transmission::socket_server server(2007);
	std::future<bool> future_server = std::async(std::launch::async, [&server]()->bool {
		server.run();
		return true;
	});

	std::mutex q_mtx;
	std::condition_variable q_cv;
	std::atomic<bool> q_flag(false);
	
	//start schedule_stop function
	auto schedule_stop = std::async(std::launch::async, [&server, &q_mtx, &q_cv, &q_flag, &requestsProcessor, &responseProcessor](){
		std::unique_lock<std::mutex> lk(q_mtx);
		q_cv.wait(lk, [&q_flag]{return q_flag.load(); });
		requestsProcessor.Stop();
		responseProcessor.Stop();
		server.stop();
	});

	//check if it's on trade available time periodically (15 min)
	auto check_quit = std::async(std::launch::async, [&q_flag, &q_cv](){
		const int MilliSecondsPerQuarter = 15 * 60 * 1000;
		sleep(MilliSecondsPerQuarter); // 允许在非交易时间 运行15分钟
		while (q_flag.load() == false)
		{
			std::time_t result = std::time(nullptr);
			struct tm * now_local = std::localtime(&result);
			int second_elapse = now_local->tm_hour * 3600 + now_local->tm_min * 60 + now_local->tm_sec;
			if (CommonUtils::IsMarketingTime(second_elapse) == false)
			{
				q_flag.store(true);
				q_cv.notify_all();
			}
		}
	});

	if (future_response_processor.get() == true){
		SYNC_LOG << "1) Shutdown Response Action processor...Success";
	}

	if (future_request_processor.get() == true){
		SYNC_LOG << "2) Shutdown Request Action processor...Success";
	}		

	if (future_server.get() == true){
		SYNC_LOG << "3) Shutdown Socket Server...Success";
	}	
}

/*
Usage: 
   AutoTrade.exe
   AutoTrade.exe replay rb1510 table_mark
*/
int main(int argc, const char* argv[]){
	StrategyPluginsLoader loader; //must be top
	auto console = spdlog::stdout_logger_mt("console");
	auto file_logger = spdlog::rotating_logger_mt("file_logger", "logs/logfile", 1048576 * 5, 3);
	auto file_debug_logger = spdlog::rotating_logger_mt("file_debug_logger", "logs/debug", 1048576 * 5, 3);
	auto file_trade_logger = spdlog::rotating_logger_mt("file_trade_logger", "logs/trade", 1048576 * 5, 3);
	file_logger->info() << "----------------------------------";
	file_logger->info() << "Lanuch AutoTrader...";
	//Test
	//RunUnitTest();

	if (argc == 6 && strcmp(argv[1], "replay") == 0){
#ifdef MUSTIMPL
		ReplayTickDataFromDB(argv[2], argv[3], argv[4], argv[5]);
#endif
	}
	else if (argc == 5 && strcmp(argv[1], "single") == 0){
		//AutoTrader single 9999021510
		StartTradeLocally(argv[2], argv[3], argv[4], "local");
	}
	else{
		LaunchAutoTrader();
	}
	return 0;
}
