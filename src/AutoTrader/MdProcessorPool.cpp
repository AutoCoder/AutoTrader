#include "stdafx.h"
#include "Strategy.h"
#include "MdProcessorPool.h"
#include "Config.h"
#include "DBWrapper.h"
#include "tradespi.h"
#include "mdspi.h"
#include "TickWrapper.h"
#include "MdProcessor.h"

MdProcessorPool* MdProcessorPool::_instance = NULL;

MdProcessorPool* MdProcessorPool::getInstance()
{
	if (_instance == NULL)
	{
		_instance = new MdProcessorPool();
	}
	return _instance;
}

MdProcessorPool::MdProcessorPool()
	:m_mdspi(nullptr)
{
}

void MdProcessorPool::SetMdSpi(CtpMdSpi* p)
{
	m_mdspi = p;
}

void MdProcessorPool::AddProcessor(const std::string& instrument, OrderTriggerBase* trigger, BaseClientSession* session){
	assert(m_mdspi);
	auto processor = std::make_shared<MdProcessor>(trigger, instrument, session, m_mdspi);

	auto& processorVec = m_processorDict[instrument];
	processorVec.push_back(processor);
}

void MdProcessorPool::recoverHistoryData(int beforeSeconds, const std::string& instrumentId)
{
	//the previous tradeDay's 1200
	const char * sqlselect = "SELECT * FROM %s.%s order by id desc limit %d;";

	char sqlbuf[512];
	SPRINTF(sqlbuf, sqlselect, Config::Instance()->DBName().c_str(), instrumentId.c_str(), beforeSeconds*2); //beforeSeconds*2  ==  n(s) * (1 call back /500ms)

	std::map<int, std::vector<std::string> > map_results;
	m_dbptr->Query(sqlbuf, map_results);

	auto& pMdProcessor = m_processorDict[instrumentId];

}

void MdProcessorPool::AppendTick(TickWrapper& info)
{
	auto processorVec = m_processorDict[info.InstrumentId()];
	for (auto proccessor : processorVec){
		if (proccessor->IsTrading())
			proccessor->AppendTick(info);
	}
}

// void MdProcessorPool::StoreStrategySequenceToDB(const std::string& instrumentID, const std::string& mark)
// {
// 	auto processorVec = m_processorDict[instrumentID]; 
// 	for (auto proccessor : processorVec){
// 		proccessor->StoreStrategySequenceToDB(mark);
// 	}
// }