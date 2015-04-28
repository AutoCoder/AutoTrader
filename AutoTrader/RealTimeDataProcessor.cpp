#include "stdafx.h"
#include "RealTimeDataProcessor.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "Strategy.h"
#include "Order.h"
#include "OrderQueue.h"

static threadsafe_queue<Order> order_queue;

RealTimeDataProcessor::RealTimeDataProcessor(Strategy* strag)
	:m_strategy(strag)
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
	if (m_DataSeq.size() >= QueueSize){
		const CThostFtdcDepthMDFieldWrapper& firstDataBlock = m_DataSeq.back();
		firstDataBlock.serializeToDB();
		m_DataSeq.pop_back();
	}

	//loop the Strategies

	if (m_strategy->TryInvoke(m_DataSeq, info)){
		order_queue.push(m_strategy->generateOrder());
	}
}