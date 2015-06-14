#ifndef MACROSS_STRATEGY_H
#define MACROSS_STRATEGY_H

#include "Strategy.h"
#include "TechVec.h"

class MACrossStratgyTechVec;
class MACrossStratgy : public Strategy
{
public:
	MACrossStratgy();
	virtual ~MACrossStratgy();

	virtual bool tryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info);
	virtual Order generateOrder();

protected:
	virtual double calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const;
	virtual MACrossStratgyTechVec* generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const;
private:
	Order* m_curOrder;
};

typedef StratgyType::MACrossStratgyType CrossStratgyType;

class MACrossStratgyTechVec : public StrategyTechVec{
public:
	MACrossStratgyTechVec(CrossStratgyType type, long long uuid, const std::string& instrumentID, const std::string& time = "", double lastprice = 0);
	virtual ~MACrossStratgyTechVec(){}

	virtual size_t ObjSize(){
		return sizeof(*this);
	}

	inline void setK3m(double input){
		m_k3m = input;
	}

	inline double K3m() const {
		return m_k3m;
	}

	inline void setK5m(double input){
		m_k5m = input;
	}

	inline double K5m() const {
		return m_k5m;
	}

	virtual void SetTickType(TickType type){
		m_ticktype = type;
	}

	virtual TickType GetTickType(){
		return m_ticktype;
	}

	virtual bool IsUpThough() const;

	virtual void serializeToDB(DBWrapper& db, const std::string& mark);

protected:
	virtual int CreateTableIfNotExists(const std::string& dbname, const std::string& tableName);
	static bool IsTableCreated;

private:
	double m_k5m; //average line for 5 minutes
	double m_k3m; //average line for 3 minutes
	TickType m_ticktype;
	char m_instrumentId[32];
	char m_time[64];// for check result 

	double m_lastprice; // for check result
	long long m_id; //time_stamp * 2  (unit : 500ms)
	StratgyType::MACrossStratgyType m_type;
};

#endif
