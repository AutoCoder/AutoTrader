#include "stdafx.h"
#include "MACrossStratgy.h"
#include "WMACrossStratgy.h"
#include "AMACrossStratgy.h"
#include "EMACrossStratgy.h"
#include "Strategy.h"
#include "RealTimeDataProcessorPool.h"
#include "config.h"
#include "DBWrapper.h"

RealTimeDataProcessorPool* RealTimeDataProcessorPool::_instance = NULL;

RealTimeDataProcessorPool* RealTimeDataProcessorPool::getInstance()
{
	if (_instance == NULL)
	{
		//static clearer clr;
		_instance = new RealTimeDataProcessorPool();
	}
	return _instance;
}

RealTimeDataProcessorPool::RealTimeDataProcessorPool()
	:m_dbptr(new DBWrapper)
{
	//construct the Strategy dict 
	Config::Instance()->CtpBrokerID();
	std::vector<StrategyMetaData> stgySet = Config::Instance()->StrategySet();
	m_dict.clear();
	for (StrategyMetaData it : stgySet){
		if (it.name == "MACross"){
			m_dict["MACross"] = std::shared_ptr<Strategy>(new MACrossStratgy(it.short_ma, it.long_ma));
		}
		else if (it.name == "WMACross"){
			m_dict["WMACross"] = std::shared_ptr<Strategy>(new WMACrossStratgy(it.short_ma, it.long_ma));
		}
		else if (it.name == "AMACross"){
			m_dict["AMACross"] = std::shared_ptr<Strategy>(new AMACrossStratgy(it.short_ma, it.long_ma));
		}
		else if (it.name == "EMACross"){
			m_dict["EMACross"] = std::shared_ptr<Strategy>(new EMACrossStratgy(it.short_ma, it.long_ma));
		}
	}

	m_processorDict.clear();

	if (Config::Instance()->RecordModeOn()){
		m_processorDict["rb1510"] = std::shared_ptr<RealTimeDataProcessor>(new RealTimeDataProcessor(nullptr, "rb1510"));
		m_processorDict["rb1511"] = std::shared_ptr<RealTimeDataProcessor>(new RealTimeDataProcessor(nullptr, "rb1511"));
	}
	else{
		//m_processorDict["rb1510"] = std::shared_ptr<RealTimeDataProcessor>(new RealTimeDataProcessor(m_dict["k3UpThroughK5"].get(), "rb1510"));
		std::string st1 = Config::Instance()->CtpStrategy("rb1510");
		std::string st2 = Config::Instance()->CtpStrategy("rb1511");
		m_processorDict["rb1510"] = std::shared_ptr<RealTimeDataProcessor>(new RealTimeDataProcessor(m_dict[st1].get(), "rb1510"));
		m_processorDict["rb1511"] = std::shared_ptr<RealTimeDataProcessor>(new RealTimeDataProcessor(m_dict[st2].get(), "rb1511"));
	}

}

void RealTimeDataProcessorPool::recoverHistoryData(int beforeSeconds, const std::string& instrumentId)
{
	//the previous tradeDay's 1200
	const char * sqlselect = "SELECT * FROM %s.%s order by id desc limit %d;";

	char sqlbuf[512];
	sprintf_s(sqlbuf, sqlselect, Config::Instance()->DBName(), instrumentId.c_str(), beforeSeconds*2); //beforeSeconds*2  ==  n(s) * (1 call back /500ms)

	std::map<int, std::vector<std::string>> map_results;
	m_dbptr->Query(sqlbuf, map_results);

	auto& pRealTimeDataProcessor = m_processorDict[instrumentId];

}

std::shared_ptr<RealTimeDataProcessor> RealTimeDataProcessorPool::GenRealTimeDataProcessor(const std::string& instrumentID)
{
	if (m_processorDict.end() == m_processorDict.find(instrumentID))
		return NULL;
	else{
		return m_processorDict[instrumentID];
	}
}


void RealTimeDataProcessorPool::FreeProcessors()
{
	for (auto item : m_processorDict){
		item.second->StoreDataToDB();
	}
}