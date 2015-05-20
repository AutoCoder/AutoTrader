#ifndef STRATEGY_H
#define STRATEGY_H

#include "ThostFtdcDepthMDFieldWrapper.h"
#include <list>

class Order;

class Strategy
{
public:
	Strategy(){}
	~Strategy(){}

	virtual bool TryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info) = 0;

	virtual Order generateOrder() = 0;

};

#endif