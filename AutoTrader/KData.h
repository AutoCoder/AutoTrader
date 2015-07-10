#ifndef K_DATA_H
#define K_DATA_H
#include <vector>
#include "TickWrapper.h"

typedef std::vector<TickWrapper> TickSet;

class KData
{
public:
	KData(const TickSet& tickset, size_t duration);
	~KData();

	void serializeToDB();

private:
	static bool IsTableCreated;
	static int CreateKDataTableIfNotExists(const std::string& dbname, const std::string& tableName);

private:
	long long m_volume;
	double m_BidPrice1;
	double m_AskPrice1;
	double m_Turnover;
	double m_LowestPrice;
	double m_HighestPrice;
	double m_OpenPrice;
	double m_PreSettlementPrice;
	double m_PreOpenInterest;
	double m_PreClosePrice;
	double m_LastPrice;
	double m_UpperLimitPrice;
	double m_LowerLimitPrice;
	std::string m_InstrumentID;
	size_t m_duration; //seconds
};

#endif