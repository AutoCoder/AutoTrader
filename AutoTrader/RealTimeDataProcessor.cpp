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

threadsafe_queue<Order> order_queue;

RealTimeDataProcessor::RealTimeDataProcessor(Strategy* strag, const std::string& InstrumentName)
	: m_strategy(strag)
	, m_Name(InstrumentName)
	, m_dbptr(new DBWrapper)
{
	recoverHistoryData(600);
}
  
RealTimeDataProcessor::~RealTimeDataProcessor()
{
	//store all item in memory into db
	for (auto iter = m_DataSeq.rbegin(); iter != m_DataSeq.rend(); iter++){
		iter->serializeToDB(*(m_dbptr.get()));
	}

	//for (auto&& iter : m_DataSeq)
	//{
	//	iter.serializeToDB();
	//}
	m_DataSeq.clear();
}

void RealTimeDataProcessor::AppendRealTimeData(CThostFtdcDepthMDFieldWrapper& info){
	//(in)front-------------back(out)
	assert(m_strategy!=NULL);
	bool triggered = m_strategy->TryInvoke(m_DataSeq, info);
	m_DataSeq.push_front(info);
#ifdef _DEBUG
	std::cerr << "> Data queue size :" << m_DataSeq.size()<< std::endl;
#endif
	if (triggered){
		order_queue.push(m_strategy->generateOrder());
	}
}

void RealTimeDataProcessor::recoverHistoryData(int beforeSeconds)
{
	//the previous tradeDay's 1200
	const char * sqlselect = "SELECT * FROM %s.%s order by id limit %d;";

	char sqlbuf[512];
	sprintf_s(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), m_Name.c_str(), beforeSeconds * 2); //beforeSeconds*2  ==  n(s) * (1 call back /500ms)

	std::map<int, std::vector<std::string>> map_results;
	m_dbptr->Query(sqlbuf, map_results);
	
	for (auto item : map_results){
		m_DataSeq.push_front(CThostFtdcDepthMDFieldWrapper::RecoverFromDB(item.second));
	}
}