#ifndef REALTIME_DATA_PROCESSOR_H
#define REALTIME_DATA_PROCESSOR_H

#include <list>
#include <vector>
#include <memory>

class CThostFtdcDepthMDFieldWrapper;
class Strategy;
class DBWrapper;

#define QueueSize 10

class RealTimeDataProcessor
{
public:
	RealTimeDataProcessor(Strategy* strag, const std::string& InstrumentName);
	~RealTimeDataProcessor();

	void AppendRealTimeData(CThostFtdcDepthMDFieldWrapper& info);
	void StoreDataToDB();

private:
	void recoverHistoryData(int beforeSeconds);

private:
	std::list<CThostFtdcDepthMDFieldWrapper> m_DataSeq;
	std::string m_Name;
	Strategy* m_strategy;
	std::shared_ptr<DBWrapper> m_dbptr;
};


#endif
