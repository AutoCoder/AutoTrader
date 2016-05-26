#ifndef REALTIME_DATA_PROCESSOR_POOL_H
#define REALTIME_DATA_PROCESSOR_POOL_H

#include <string>
#include <map>
#include <set>
#include "MdProcessor.h"
#include <memory>

class DBWrapper;
class CtpTradeSpi;
class BaseClientSession;
class CtpMdSpi;
//not for multi-thread: this singleton should be called on main()
class MdProcessorPool
{
public:
	~MdProcessorPool(){};
	static MdProcessorPool* getInstance();

public:
	void SetMdSpi(CtpMdSpi* p);
	void AddProcessor(const std::string& instrument, OrderTriggerBase* trigger, BaseClientSession* session);
	void AppendTick(TickWrapper& info);
	void StoreStrategySequenceToDB(const std::string& instrumentID, const std::string& mark);

private:
	MdProcessorPool();
	MdProcessorPool(const MdProcessorPool&) = delete;
	MdProcessorPool& operator=(const MdProcessorPool &) = delete;
	static MdProcessorPool *_instance;

	//class clearer
	//{
	//public:
	//	clearer(){}
	//	~clearer()
	//	{
	//		if (MdProcessorPool::getInstance())
	//		{
	//			delete MdProcessorPool::getInstance();
	//		}
	//	}
	//};

private:
	void recoverHistoryData(int beforeSeconds, const std::string& instrumentId);

private:
	std::shared_ptr<DBWrapper> m_dbptr;
	std::map<std::string/*instrument*/, std::vector<std::shared_ptr<MdProcessor> > >  m_processorDict;
	CtpMdSpi*                  m_mdspi;
};

#endif