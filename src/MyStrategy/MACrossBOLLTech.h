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

	void setBollTech(const BOLLTech& bt){
		m_bollTech = bt;
	}

	BOLLTech BollTech() const {
		return m_bollTech;
	}

	void setShortMA(double input){
		m_maTech.mShortMAVal = input;
	}

	double ShortMA() const {
		return m_maTech.mShortMAVal;
	}

	void setLongMA(double input){
		m_maTech.mLongMAVal = input;
	}

	inline double LongMA() const {
		return m_maTech.mLongMAVal;
	}

	virtual	void serializeToDB(DBWrapper& db, const std::string& mark);
	virtual bool IsTriggerPoint() const;

protected:
	static int CreateTableIfNotExists(const std::string& dbname, const std::string& tableName);
	static bool IsTableCreated;

private:
	StratgyType::MACrossStratgyType m_type;
	long long m_id; //time_stamp * 2  (unit : 500ms)
	TickType m_ticktype;
	double m_lastprice; // for check result
	size_t m_shortMA;
	size_t m_longMA;
	char m_instrumentId[32];
	char m_time[64];// for check result 
	MATech m_maTech;	
	BOLLTech m_bollTech;
};

#endif