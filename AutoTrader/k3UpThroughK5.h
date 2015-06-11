#ifndef K3_UPTHROUGH_K5_H
#define K3_UPTHROUGH_K5_H

#include "Strategy.h"
#include "TechVec.h"

class Order;

class k3UpThroughK5 : public Strategy
{
public:
	k3UpThroughK5();
	~k3UpThroughK5();

	virtual bool TryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info);

	virtual Order generateOrder();

private:
	double calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const;

private:
	Order* m_curOrder;
};

class k3UpThroughK5TechVec : public StrategyTechVec{
public:
	k3UpThroughK5TechVec(long long uuid, const std::string& instrumentID, const std::string& time="", double lastprice=0);
	virtual ~k3UpThroughK5TechVec(){}

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

	std::string m_instrumentId;
	std::string m_time; // for check result 
	double m_lastprice; // for check result
	long long m_id; //time_stamp * 2  (unit : 500ms)
};

#endif