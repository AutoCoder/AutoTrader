#include "stdafx.h"
#include "Strategy.h"
#include "RealTimeDataProcessorPool.h"

RealTimeDataProcessorPool* RealTimeDataProcessorPool::_instance = NULL;

RealTimeDataProcessorPool* RealTimeDataProcessorPool::getInstance()
{
	if (_instance == NULL)
	{
		static clearer clr;
		_instance = new RealTimeDataProcessorPool();
	}
	return _instance;
}

RealTimeDataProcessorPool::RealTimeDataProcessorPool()
{
	//construct the Strategy dict
	m_dict.clear();
	m_dict["k3UpThroughK5"] = std::shared_ptr<Strategy>(new k3UpThroughK5());

	m_processorDict.clear();
	m_processorDict["rb1510"] = std::shared_ptr<RealTimeDataProcessor>(new RealTimeDataProcessor(m_dict["k3UpThroughK5"].get(), "rb1510"));
	m_processorDict["rb1511"] = std::shared_ptr<RealTimeDataProcessor>(new RealTimeDataProcessor(m_dict["k3UpThroughK5"].get(), "rb1511"));
}

std::shared_ptr<RealTimeDataProcessor> RealTimeDataProcessorPool::GenRealTimeDataProcessor(const std::string& instrumentID)
{
	if (m_processorDict.end() == m_processorDict.find(instrumentID))
		return NULL;
	else{
		return m_processorDict[instrumentID];
	}
}