#ifndef MACROSS_BOLL_TECH_H
#define MACROSS_BOLL_TECH_H

#include "TechVec.h"
#include "BOLLTech.h"
#include "MATech.h"
#include "Strategy.h"

class MACrossBOLLTech : public StrategyTech
{
public:
	MACrossBOLLTech(CrossStratgyType type, size_t shortMA, size_t longMA, long long uuid, const std::string& instrumentID, const std::string& time = "", double lastprice = 0);
	virtual ~MACrossBOLLTech();

	virtual void SetTickType(TickType type){
		m_ticktype = type;
	}

	virtual TickType GetTickType(){
		return m_ticktype;
	}

	virtual size_t ObjSize(){
		return sizeof(*this);
	}

	virtual	void serializeToDB(DBWrapper& db, const std::string& mark);
	virtual bool IsTriggerPoint() const;

protected:
	static int CreateTableIfNotExists(const std::string& dbname, const std::string& tableName);
	static bool IsTableCreated;

private:
	BOLLTech m_bollTech;
	MATech m_maTech;

	char m_instrumentId[32];
	char m_time[64];// for check result 
	double m_lastprice; // for check result
	long long m_id; //time_stamp * 2  (unit : 500ms)
	StratgyType::MACrossStratgyType m_type;
	TickType m_ticktype;
	size_t m_shortMA;
	size_t m_longMA;
};

#endif