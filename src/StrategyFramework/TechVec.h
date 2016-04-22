#ifndef TECH_VEC_H_
#define TECH_VEC_H_

#include <string>
#include "stdafx.h"

enum class TickType{
	Commom = 0x0,
	BuyPoint = 0x1,
	SellPoint = 0x2,
};

class DBWrapper;

class STRATEGY_API StrategyTech{
public:
	StrategyTech(){}
	virtual ~StrategyTech(){}

	virtual	void serializeToDB(DBWrapper& db, const std::string& mark) = 0;
	virtual void SetTickType(TickType type) = 0;
	virtual std::string ToJson() const { return ""; };

	virtual TickType GetTickType() = 0;
	virtual size_t ObjSize() = 0;
	virtual bool IsTriggerPoint() const = 0;
};

#endif