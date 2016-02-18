#include "stdafx.h"
#include "RealTimeDataProcessor.h"
#include "TickWrapper.h"
#include "KData.h"
#include "Order.h"
#include "ThreadSafeQueue.h"
#include <iostream>
#include <assert.h>
#include "Config.h"
#include "DBWrapper.h"
#include "CommonUtils.h"
#include "spdlog/spdlog.h"
#include "crossplatform.h"
#include "ClientSession.h"
#include "OrderTrigger.h"

extern std::atomic<bool> g_reply;
extern threadsafe_queue<Order> order_queue;

#define UseKDataToInvoke 1

RealTimeDataProcessor::RealTimeDataProcessor(OrderTriggerBase* trigger, const std::string& InstrumentName, ClientSession* owner)
	: m_Name(InstrumentName)
	, m_trigger(trigger)
	, m_owner(owner)
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
	if (m_trigger){
		Order ord;
#ifdef UseKDataToInvoke
		bool triggered = m_trigger->tryInvoke(m_DataSeq, m_KDataVec, m_TickSet60, info, ord);
#else
		bool triggered = m_trigger->tryInvoke(m_DataSeq, info, ord);
#endif
		if (triggered){
			ord.SetTriggerTick(info.toTimeStamp());

			if (m_owner){
				m_owner->AppendOrder(ord);
			}
		}
	}
	if (m_owner)
		m_owner->InformClientViewer(info);

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
	SPRINTF(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), m_Name.c_str(), beforeSeconds * 2); //beforeSeconds*2  ==  n(s) * (1 call back /500ms)

	std::map<int, std::vector<std::string>> map_results;
	m_dbptr->Query(sqlbuf, map_results);
	
	for (auto item : map_results){
		m_DataSeq.push_front(TickWrapper::RecoverFromDB(item.second));
	}

}

bool RealTimeDataProcessor::IsTrading() const 
{ 
	return m_owner ? m_owner->IsTrading() : false;
}
