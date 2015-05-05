#ifndef REALTIME_DATA_PROCESSOR_POOL_H
#define REALTIME_DATA_PROCESSOR_POOL_H
#include <string>
#include <map>
#include <set>
#include "RealTimeDataProcessor.h"
class DBWrapper;

//not for multi-thread: this singleton should be called on main()
class RealTimeDataProcessorPool
{
public:
	~RealTimeDataProcessorPool(){};
	static RealTimeDataProcessorPool* getInstance();

public:
	std::shared_ptr<RealTimeDataProcessor> GenRealTimeDataProcessor(const std::string& instrumentID);

private:
	RealTimeDataProcessorPool();
	RealTimeDataProcessorPool(const RealTimeDataProcessorPool&){};
	RealTimeDataProcessorPool& operator=(const RealTimeDataProcessorPool &){};
	static RealTimeDataProcessorPool *_instance;

	class clearer
	{
	public:
		clearer(){}
		~clearer()
		{
			if (RealTimeDataProcessorPool::getInstance())
			{
				delete RealTimeDataProcessorPool::getInstance();
			}
		}
	};

private:
	void recoverHistoryData(int beforeSeconds, const std::string& instrumentId);

private:
	std::shared_ptr<DBWrapper> m_dbptr;
	std::map<std::string, std::shared_ptr<Strategy>> m_dict;
	std::map<std::string, std::shared_ptr<RealTimeDataProcessor>> m_processorDict;
};

#endif