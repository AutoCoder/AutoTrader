#ifndef REALTIME_DATA_PROCESSOR_H
#define REALTIME_DATA_PROCESSOR_H

#include <list>
#include <vector>
#include <memory>

class TickWrapper;
class OrderTriggerBase;
class DBWrapper;
class KData;
class BaseClientSession;
#define QueueSize 10

class RealTimeDataProcessor
{
public:
	//if strag == nullptr, that mean RealTimeDataProcessor is in data-recording mode. 
	RealTimeDataProcessor(OrderTriggerBase* trigger, const std::string& InstrumentName, BaseClientSession* owner, bool replay = false);
	~RealTimeDataProcessor();

	void AppendRealTimeData(TickWrapper& info);
	void StoreDataToDB();
	void StoreStrategySequenceToDB(const std::string& suggestTableName = "");
	std::string Instrument() const { return m_Name; }
	bool IsTrading() const;

private:
	void recoverHistoryData(int beforeSeconds);

private:
	std::list<TickWrapper>		m_DataSeq;
	std::vector<TickWrapper>	m_TickSet60;
	std::vector<KData>		    m_KDataVec;
	std::string					m_Name;
	OrderTriggerBase*			m_trigger;
	BaseClientSession*			    m_owner;
	std::shared_ptr<DBWrapper>	m_dbptr;
};


#endif
