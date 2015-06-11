#ifndef K3AVETHOUGH5AVE_H
#define K3AVETHOUGH5AVE_H


#include "Strategy.h"
#include "TechVec.h"

class K3AVEThoughK5AVE : public Strategy
{
public:
	K3AVEThoughK5AVE();
	~K3AVEThoughK5AVE();
	virtual bool TryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info);

	virtual Order generateOrder();

private:
	double calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const;

private:
	Order* m_curOrder;
};


class K3AVEThoughK5AVETechVec: public StrategyTechVec{
public:
	K3AVEThoughK5AVETechVec(long long uuid, const std::string& instrumentID, const std::string& time = "", double lastprice = 0);
	virtual ~K3AVEThoughK5AVETechVec(){}

	virtual size_t ObjSize(){
		return sizeof(*this);
	}

	inline void setK3m(double input){
		m_k3closepriceave = input;
	}

	inline double K3m() const {
		return m_k3closepriceave;
	}

	inline void setK5m(double input){
		m_k5closepriceave = input;
	}

	inline double K5m() const {
		return m_k5closepriceave;
	}

	virtual void SetTickType(TickType type){
		m_ticktype = type;
	}

	virtual TickType GetTickType(){
		return m_ticktype;
	}

	virtual void serializeToDB(DBWrapper& db, const std::string& mark);

	virtual bool IsUpThough() const;

protected:
	virtual int CreateTableIfNotExists(const std::string& dbname, const std::string& tableName);
	static bool IsTableCreated;
private:
	double m_k5closepriceave; //average line for 5 minutes
	double m_k3closepriceave; //average line for 3 minutes
	TickType m_ticktype;

	char m_instrumentId[32];

	long long m_id; //time_stamp * 2  (unit : 500ms)

	char m_time[64];// for check result 
	double m_lastprice; // for check result
};

#endif