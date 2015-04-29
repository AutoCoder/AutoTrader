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
	RealTimeDataProcessor(Strategy* strag, const std::string& InstrumentName);
	~RealTimeDataProcessor();

	void AppendRealTimeData(CThostFtdcDepthMDFieldWrapper& info);

private:
	std::list<CThostFtdcDepthMDFieldWrapper> m_DataSeq;
	std::string m_Name;
	Strategy* m_strategy;
};


#endif
