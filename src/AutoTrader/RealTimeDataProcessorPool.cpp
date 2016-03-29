#include "stdafx.h"
#include "Strategy.h"
#include "RealTimeDataProcessorPool.h"
#include "Config.h"
#include "DBWrapper.h"
#include "tradespi.h"
#include "mdspi.h"
#include "TickWrapper.h"
#include "RealTimeDataProcessor.h"

RealTimeDataProcessorPool* RealTimeDataProcessorPool::_instance = NULL;

RealTimeDataProcessorPool* RealTimeDataProcessorPool::getInstance()
{
	if (_instance == NULL)
	{
		_instance = new RealTimeDataProcessorPool();
	}
	return _instance;
}

RealTimeDataProcessorPool::RealTimeDataProcessorPool()
	:m_mdspi(nullptr)
{
}

void RealTimeDataProcessorPool::SetMdSpi(CtpMdSpi* p)
{
	m_mdspi = p;
}

void RealTimeDataProcessorPool::AddProcessor(const std::string& instrument, OrderTriggerBase* trigger, BaseClientSession* session){
	assert(m_mdspi);
	auto processor = std::make_shared<RealTimeDataProcessor>(trigger, instrument, session, m_mdspi->GetTickVec(instrument));

	auto& processorVec = m_processorDict[instrument];
	processorVec.push_back(processor);
}

void RealTimeDataProcessorPool::recoverHistoryData(int beforeSeconds, const std::string& instrumentId)
{
	//the previous tradeDay's 1200
	const char * sqlselect = "SELECT * FROM %s.%s order by id desc limit %d;";

	char sqlbuf[512];
	SPRINTF(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), instrumentId.c_str(), beforeSeconds*2); //beforeSeconds*2  ==  n(s) * (1 call back /500ms)

	std::map<int, std::vector<std::string> > map_results;
	m_dbptr->Query(sqlbuf, map_results);

	auto& pRealTimeDataProcessor = m_processorDict[instrumentId];

}


void RealTimeDataProcessorPool::StoreCachedData()
{
	//for (auto item : m_processorDict){
	//	item.second->StoreDataToDB();
	//}

}

void RealTimeDataProcessorPool::AppendRealTimeData(TickWrapper& info)
{
	auto processorVec = m_processorDict[info.InstrumentId()];
	for (auto proccessor : processorVec){
		if (proccessor->IsTrading())
			proccessor->AppendRealTimeData(info);
	}
}

void RealTimeDataProcessorPool::StoreStrategySequenceToDB(const std::string& instrumentID, const std::string& mark)
{
	auto processorVec = m_processorDict[instrumentID]; 
	for (auto proccessor : processorVec){
		if (auto sp = proccessor.lock()){
			sp->StoreStrategySequenceToDB(mark);
		}
	}
}