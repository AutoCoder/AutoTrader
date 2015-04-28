#ifndef REALTIME_DATA_PROCESSOR_H
#define REALTIME_DATA_PROCESSOR_H

#include <list>
#include <vector>
#include <memory>

class CThostFtdcDepthMDFieldWrapper;
class Strategy;

#define QueueSize 10

class RealTimeDataProcessor
{
public:
	RealTimeDataProcessor(Strategy* strag);
	~RealTimeDataProcessor();

	void AppendRealTimeData(CThostFtdcDepthMDFieldWrapper& info);

private:
	std::list<CThostFtdcDepthMDFieldWrapper> m_DataSeq;
	std::shared_ptr<Strategy> m_strategy;
};


#endif
