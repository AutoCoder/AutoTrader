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
class CtpMdSpi;

class MdProcessor
{
public:
	//if strag == nullptr, that mean MdProcessor is in data-recording mode. 
	MdProcessor(OrderTriggerBase* trigger, const std::string& InstrumentName, BaseClientSession* owner, CtpMdSpi* spi);

	/**
	 ** Use stored md in db, not online md.
	**/
	MdProcessor(OrderTriggerBase* trigger, const std::string& InstrumentName, BaseClientSession* owner, std::vector<TickWrapper>& tickVec, std::vector<KData>& kdataVec, std::vector<TickWrapper>& tick60);
	
	~MdProcessor();

	void AppendTick(TickWrapper& info);
	void StoreStrategySequenceToDB(const std::string& suggestTableName = "");
	std::string Instrument() const { return m_Name; }
	bool IsTrading() const;

private:
	void recoverHistoryData(int beforeSeconds);

private:
	std::vector<TickWrapper>   &m_TickVec;
	std::vector<KData>		   &m_KDataVec;
	std::vector<TickWrapper>   &m_TickSet60;

	std::string					m_Name;
	OrderTriggerBase*			m_trigger;
	BaseClientSession*			m_owner;
	std::shared_ptr<DBWrapper>	m_dbptr;
	bool                        m_replay;
};


#endif
