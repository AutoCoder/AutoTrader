#pragma once
#include "Order.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include <list>

class Strategy
{
public:
	Strategy();
	~Strategy();

	virtual bool TryInvoke(std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info) = 0;

	virtual Order generateOrder() = 0;

	double calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const;
};

class k3UpThroughK5 : public Strategy
{
public:
	k3UpThroughK5();
	~k3UpThroughK5();

	virtual bool TryInvoke(std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info);

	virtual Order generateOrder();

private:
	Order* m_curOrder;
};