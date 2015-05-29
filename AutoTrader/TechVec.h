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
	StrategyTechVec(){}
	virtual ~StrategyTechVec(){}

	virtual	void serializeToDB(DBWrapper& db, const std::string& mark) = 0;
	virtual void SetTickType(TickType type) = 0;
	virtual TickType GetTickType() = 0;
	virtual size_t ObjSize() = 0;
};

#endif