#include "stdafx.h"
#include "RealTimeDataProcessor.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "Strategy.h"
#include "Order.h"
#include "OrderQueue.h"
#include <iostream>
#include <assert.h>
#include "config.h"
#include "DBWrapper.h"
#include "spdlog/spdlog.h"

extern std::atomic<bool> g_reply;
extern threadsafe_queue<Order> order_queue;

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
	//store Tick data in memory into db
	for (auto iter = m_DataSeq.rbegin(); iter != m_DataSeq.rend(); iter++){
		iter->serializeToDB(*(m_dbptr.get()));
	}
}

void RealTimeDataProcessor::StoreStrategySequenceToDB(const std::string& mark)
{
	//store Strategy data in memory into db
	for (auto iter = m_DataSeq.rbegin(); iter != m_DataSeq.rend(); iter++){
		if (iter->m_techvec != nullptr)
			iter->m_techvec->serializeToDB(*(m_dbptr.get()), mark);
	}
}

void RealTimeDataProcessor::AppendRealTimeData(CThostFtdcDepthMDFieldWrapper& info){
	//(in)front-------------back(out)
	// if m_strategy == nullptr, that means RealTimeDataProcessor is in data-recording mode
	if (m_strategy){
		bool triggered = m_strategy->TryInvoke(m_DataSeq, info);

		if (triggered)
			order_queue.push(m_strategy->generateOrder());
	}
	m_DataSeq.push_front(info);

#ifdef _DEBUG
	spdlog::get("console")->info() << "> Data queue size :" << m_DataSeq.size();
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
		m_DataSeq.push_front(CThostFtdcDepthMDFieldWrapper::RecoverFromDB(item.second));
	}

}