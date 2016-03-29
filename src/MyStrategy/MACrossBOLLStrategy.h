#ifndef MACROSSBOLL_STRATEGY_H
#define MACROSSBOLL_STRATEGY_H

#include <list>
#include <vector>

#include "stdafx.h"
#include "Strategy.h"
#include "MACrossBOLLTech.h"
#include "TechVec.h"
#include "TickWrapper.h"
#include "KData.h"
#include "Order.h"

struct BOLLTech;

class MACrossBOLLStrategy
{
public:
	MACrossBOLLStrategy(size_t short_ma, size_t long_ma, size_t boll_period);
	virtual ~MACrossBOLLStrategy();

	virtual bool tryInvoke(const std::vector<TickWrapper>& data, TickWrapper& info);
	virtual bool tryInvoke(const std::vector<TickWrapper>& tickdata, const std::vector<KData>& data, std::vector<TickWrapper> curmindata, TickWrapper& info);

	Order GetCurOrder() const;

protected:
	virtual BOLLTech calculateBoll(const std::vector<TickWrapper>& data, const TickWrapper& current, size_t seconds) const;
	virtual BOLLTech calculateBoll(const std::vector<KData>& data, const KData& current, size_t mins) const;
	virtual double calculateK(const std::vector<TickWrapper>& data, const TickWrapper& current, size_t seconds) const;
	virtual double calculateK(const std::vector<KData>& data, const KData& current, size_t mins) const;
	//virtual MACrossBOLLTech* generateTechVec(const TickWrapper& info) const;

protected:
	size_t m_shortMA;
	size_t m_longMA;
	size_t m_bollperiod;

private:
	Order* m_curOrder;
};

#endif