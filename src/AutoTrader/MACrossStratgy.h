#ifndef MACROSS_STRATEGY_H
#define MACROSS_STRATEGY_H

#include "Strategy.h"
#include "TechVec.h"
#include "MATech.h"

class MACrossTech;
class IPositionControl;
class MACrossStratgy : public Strategy
{
public:
	MACrossStratgy(size_t short_ma, size_t long_ma, IPositionControl* pctl);
	virtual ~MACrossStratgy();

	virtual bool tryInvoke(const std::list<TickWrapper>& data, TickWrapper& info);
	virtual bool tryInvoke(const std::list<TickWrapper>& tickdata, const std::vector<KData>& data, std::vector<TickWrapper> curmindata, TickWrapper& info);
	virtual bool generateOrder(Order& out);
	virtual IPositionControl* getAccountMgr();

protected:
	virtual double calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds) const;
	virtual double calculateK(const std::vector<KData>& data, const KData& current, size_t mins) const;
	virtual MACrossTech* generateTechVec(const TickWrapper& info) const; 

protected:
	size_t m_shortMA;
	size_t m_longMA;

private:
	Order* m_curOrder;
	IPositionControl* m_posControl;
};

class MACrossTech : public StrategyTech{
public:
	//MACrossTech(){};
	MACrossTech(CrossStratgyType type, size_t shortMA, size_t longMA, long long uuid, const std::string& instrumentID, const std::string& time = "", double lastprice = 0);
	virtual ~MACrossTech(){}

	virtual size_t ObjSize(){
		return sizeof(*this);
	}

	inline void setShortMA(double input){
		m_ma_tech.mShortMAVal = input;
	}

	inline double ShortMA() const {
		return m_ma_tech.mShortMAVal;
	}

	inline void setLongMA(double input){
		m_ma_tech.mLongMAVal = input;
	}

	inline double LongMA() const {
		return m_ma_tech.mLongMAVal;
	}

	virtual void SetTickType(TickType type){
		m_ticktype = type;
	}
	
	virtual TickType GetTickType(){
		return m_ticktype;
	}

	virtual bool IsTriggerPoint() const;

	virtual void serializeToDB(DBWrapper& db, const std::string& mark);

protected:
	static int CreateTableIfNotExists(const std::string& dbname, const std::string& tableName);
	static bool IsTableCreated;

private:
	MATech m_ma_tech;
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
