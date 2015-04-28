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
	m_dict["rb1510"] = std::shared_ptr<RealTimeDataProcessor>(new RealTimeDataProcessor(new k3UpThroughK5()));
}

std::shared_ptr<RealTimeDataProcessor> RealTimeDataProcessorPool::GenRealTimeDataProcessor(const std::string& instrumentID)
{
	if (m_dict.find(instrumentID) == m_dict.end())
		return NULL;
	else{
		return m_dict[instrumentID];
	}
}