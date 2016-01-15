#ifndef MACROSSBOLL_STRATEGY_H
#define MACROSSBOLL_STRATEGY_H

#include "stdafx.h"
#include "Strategy.h"
#include "MACrossBOLLTech.h"
#include "TechVec.h"

struct BOLLTech;
class IPositionControl;
class STRATEGY_API MACrossBOLLStrategy : public Strategy
{
public:
	MACrossBOLLStrategy(size_t short_ma, size_t long_ma, size_t boll_period, IPositionControl* accountMgr = nullptr);
	virtual ~MACrossBOLLStrategy();

	virtual bool tryInvoke(const std::list<TickWrapper>& data, TickWrapper& info);
	virtual bool tryInvoke(const std::list<TickWrapper>& tickdata, const std::vector<KData>& data, std::vector<TickWrapper> curmindata, TickWrapper& info);
	virtual bool generateOrder(Order& out);
	Order GetCurOrder() const;

protected:
	virtual BOLLTech calculateBoll(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds) const;
	virtual BOLLTech calculateBoll(const std::vector<KData>& data, const KData& current, size_t mins) const;
	virtual double calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds) const;
	virtual double calculateK(const std::vector<KData>& data, const KData& current, size_t mins) const;
	//virtual MACrossBOLLTech* generateTechVec(const TickWrapper& info) const;

protected:
	size_t m_shortMA;
	size_t m_longMA;
	size_t m_bollperiod;

private:
	Order* m_curOrder;
	IPositionControl* m_posControl;

};

#endif