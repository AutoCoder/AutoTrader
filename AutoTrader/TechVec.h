#ifndef TECH_VEC_H_
#define TECH_VEC_H_

#include <string>

enum class TickType{
	Commom = 0x0,
	BuyPoint = 0x1,
	SellPoint = 0x2,
};

class DBWrapper;

class StrategyTechVec{
public:
	explicit StrategyTechVec(long long uuid, const std::string& instrumentID)
		:m_id(uuid), m_instrumentId(instrumentID)
	{}
	virtual ~StrategyTechVec(){}
	virtual	void serializeToDB(DBWrapper& db) = 0;
	virtual void SetTickType(TickType type) = 0;
protected:
	std::string m_instrumentId;
	long long m_id; //time_stamp * 2  (unit : 500ms)
};

class k3UpThroughK5TechVec : public StrategyTechVec{
public:
	k3UpThroughK5TechVec(long long uuid, const std::string& instrumentID);
	~k3UpThroughK5TechVec(){}

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

	virtual void serializeToDB(DBWrapper& db);

private:
	double m_k5m; //average line for 5 minutes
	double m_k3m; //average line for 3 minutes
	TickType m_ticktype;
};

class TechVec{
public:

	explicit TechVec(long long uuid, const std::string& instrumentID); // ms must be 500ms or 0ms
	~TechVec();

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

	void SetTickType(TickType type, int idx){
		m_ticktype[idx] = type;
	}

	void serializeToDB(DBWrapper& db);

private:
	std::string m_instrumentId;
	long long m_id; //time_stamp * 2  (unit : 500ms)
	double m_k5m; //average line for 5 minutes
	double m_k3m; //average line for 3 minutes
	TickType m_ticktype[8];
};

#endif