#include "stdafx.h"
#include "RealTimeDataProcessor.h"
#include "TickWrapper.h"
#include "KData.h"
#include "Strategy.h"
#include "Order.h"
#include "OrderQueue.h"
#include <iostream>
#include <assert.h>
#include "config.h"
#include "DBWrapper.h"
#include "CommonUtils.h"
#include "spdlog/spdlog.h"

extern std::atomic<bool> g_reply;
extern threadsafe_queue<Order> order_queue;

#define UseKDataToInvoke 1

RealTimeDataProcessor::RealTimeDataProcessor(Strategy* strag, const std::string& InstrumentName)
	: m_strategy(strag)
	, m_Name(InstrumentName)
	, m_dbptr(new DBWrapper)
{
	if (!g_reply)
		recoverHistoryData(600);
}
  
RealTimeDataProcessor::~RealTimeDataProcessor()
{
	m_DataSeq.clear();
}

void RealTimeDataProcessor::StoreDataToDB()
{
	std::vector<TickWrapper> oneMinuteVec;
	//store Tick data in memory into db
	for (auto iter = m_DataSeq.rbegin(); iter != m_DataSeq.rend(); iter++){
		iter->serializeToDB(*(m_dbptr.get()));
	}

	//store 1-Min K-data in memory into db
	for (auto iter = m_KDataVec.begin(); iter != m_KDataVec.end(); iter++){
		iter->serializeToDB(*(m_dbptr.get()));
	}
}

void RealTimeDataProcessor::StoreStrategySequenceToDB(const std::string& mark)
{
	SYNC_PRINT << "Start to store db...";
	//store Strategy data in memory into db
	long long pre_uuid = 0;
	TickType pre_type = TickType::Commom;
	for (auto iter = m_DataSeq.rbegin(); iter != m_DataSeq.rend(); iter++){
		if (iter->m_techvec != nullptr && iter->m_techvec->GetTickType() != TickType::Commom && iter->toTimeStamp() != (pre_uuid + 1) && iter->m_techvec->GetTickType() != pre_type){
			pre_uuid = iter->toTimeStamp();
			pre_type = iter->m_techvec->GetTickType();
			iter->m_techvec->serializeToDB(*(m_dbptr.get()), mark);
		}
	}
	SYNC_PRINT << "End to store db.";
}

//main thread
void RealTimeDataProcessor::AppendRealTimeData(TickWrapper& info){
	//(in)front-------------back(out)
	// if m_strategy == nullptr, that means RealTimeDataProcessor is in data-recording mode
	if (m_strategy){
#ifdef UseKDataToInvoke
		bool triggered = m_strategy->tryInvoke(m_DataSeq, m_KDataVec, m_TickSet60, info);
#else
		bool triggered = m_strategy->tryInvoke(m_DataSeq, info);
#endif
		if (triggered){
			Order ord;
			//for now, only permit order_queue has one item.
			if (order_queue.empty() && m_strategy->generateOrder(ord)){
				order_queue.push(ord);
			}
		}
			
	}
	m_DataSeq.push_front(info);

	//construct 1-minutes k-line intermediate data
	if (m_TickSet60.empty()){
		m_TickSet60.push_back(info);
	}
	else{
		if (CommonUtils::InSameMinute(info.Time(), m_TickSet60.front().Time())){
			m_TickSet60.push_back(info);
		}
		else{ // the comming tick data is in next minutes
			KData k1m(m_TickSet60, 60);
			m_KDataVec.push_back(k1m);

			m_TickSet60.clear();
		}
	}
#ifdef SHOW_PROGRESS
	SYNC_PRINT << "> Data queue size :" << m_DataSeq.size();
#endif
}

void RealTimeDataProcessor::recoverHistoryData(int beforeSeconds)
{
	//the previous tradeDay's 1200
	const char * sqlselect = "select * from (select * from %s.%s order by id desc limit %d) as tbl order by tbl.id;";

	char sqlbuf[512];
	sprintf_s(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), m_Name.c_str(), beforeSeconds * 2); //beforeSeconds*2  ==  n(s) * (1 call back /500ms)

	std::map<int, std::vector<std::string>> map_results;
	m_dbptr->Query(sqlbuf, map_results);
	
	for (auto item : map_results){
		m_DataSeq.push_front(TickWrapper::RecoverFromDB(item.second));
	}

}
