#pragma once
#include "Strategy.h"
#include "MACrossBOLLTech.h"
#include "TechVec.h"

struct BOLLTech;

class MACrossBOLLStrategy : public Strategy
{
public:
	MACrossBOLLStrategy(size_t short_ma, size_t long_ma, size_t boll_period);
	virtual ~MACrossBOLLStrategy();

	virtual bool tryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info);
	virtual Order generateOrder();

protected:
	virtual BOLLTech calculateBoll(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds, double ma_val) const;
	virtual double calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const;
	//virtual MACrossBOLLTech* generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const;

protected:
	size_t m_shortMA;
	size_t m_longMA;
	size_t m_bollperiod;

private:
	Order* m_curOrder;


};

