#include "stdafx.h"
#include "RealTimeDataProcessor.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "Strategy.h"
#include "Order.h"
#include "OrderQueue.h"
#include <iostream>
#include <assert.h>

static threadsafe_queue<Order> order_queue;

RealTimeDataProcessor::RealTimeDataProcessor(Strategy* strag, const std::string& InstrumentName)
	: m_strategy(strag)
	, m_Name(InstrumentName)
{
}
  
RealTimeDataProcessor::~RealTimeDataProcessor()
{
	//store all item in memory into db
	for (auto&& iter : m_DataSeq)
	{
		iter.serializeToDB();
	}
	m_DataSeq.clear();
}

void RealTimeDataProcessor::AppendRealTimeData(CThostFtdcDepthMDFieldWrapper& info){
	//(in)front-------------back(out)
	static int size = 1;
	assert(m_strategy!=NULL);
	bool triggered = m_strategy->TryInvoke(m_DataSeq, info);
	m_DataSeq.push_front(info);
	std::cerr << "> Data queue size :" << size++ << std::endl;
	if (triggered){
		order_queue.push(m_strategy->generateOrder());
	}
}