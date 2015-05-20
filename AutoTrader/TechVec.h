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

#endif