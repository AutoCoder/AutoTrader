#ifndef K_DATA_H
#define K_DATA_H

#include "stdafx.h"
#include <vector>
#include <map>
#include "TickWrapper.h"

typedef std::vector<TickWrapper> TickSet;
class DBWrapper;

class STRATEGY_API KData
{
public:
	KData(const TickSet& tickset, size_t duration);
	~KData();

	void serializeToDB(DBWrapper& db);

	double TurnOver() const{
		return m_Turnover;
	}

	long long Volume() const{
		return m_volume;
	}

	double LastPrice() const{
		return m_LastPrice;
	}

	long long Timestamp() const{
		return m_timestamp;
	}

private:
	static bool IsTableCreated;
	static int CreateKDataTableIfNotExists(const std::string& dbname, const std::string& tableName);
	static std::map<std::string, bool> TableIsCreatedMap;

private:
	long long m_volume;
	double m_BidPrice1;
	double m_BidVolume1;
	double m_AskPrice1;
	double m_AskVolume1;
	double m_Turnover;
	double m_LowestPrice;
	double m_HighestPrice;
	double m_OpenPrice;
	double m_ClosePrice;
	double m_PreSettlementPrice;
	double m_SettlementPrice;

	double m_PreClosePrice;
	double m_LastPrice;
	double m_UpperLimitPrice;
	double m_LowerLimitPrice;
	double m_averagePrice;
	long long m_timestamp;
	std::string m_InstrumentID;
	std::string m_time;
	size_t m_duration; //seconds
};

#endif