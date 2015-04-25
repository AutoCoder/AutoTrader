#pragma once
#include "Order.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include <queue>

class Strategy
{
public:
	Strategy();
	~Strategy();

	virtual bool check(const std::queue<CThostFtdcDepthMDFieldWrapper>& data) = 0;

	virtual Order generateOrder() = 0;
};

class k3UpThroughK5 : public Strategy
{
public:
	k3UpThroughK5();
	~k3UpThroughK5();

	virtual bool check(const std::queue<CThostFtdcDepthMDFieldWrapper>& data);

	virtual Order generateOrder();
private:
	Order* m_curOrder;
};