#ifndef REALTIME_DATA_PROCESSOR_H
#define REALTIME_DATA_PROCESSOR_H

#include <list>
#include <vector>
#include <memory>

class TickWrapper;
class Strategy;
class DBWrapper;

#define QueueSize 10

class RealTimeDataProcessor
{
public:
	//if strag == nullptr, that mean RealTimeDataProcessor is in data-recording mode. 
	RealTimeDataProcessor(Strategy* strag, const std::string& InstrumentName);
	~RealTimeDataProcessor();

	void AppendRealTimeData(TickWrapper& info);
	void StoreDataToDB();
	void StoreStrategySequenceToDB(const std::string& suggestTableName = "");

private:
	void recoverHistoryData(int beforeSeconds);

private:
	std::list<TickWrapper> m_DataSeq;
	std::string m_Name;
	Strategy* m_strategy;
	std::shared_ptr<DBWrapper> m_dbptr;
};


#endif
