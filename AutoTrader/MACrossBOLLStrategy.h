#ifndef MACROSSBOLL_STRATEGY_H
#define MACROSSBOLL_STRATEGY_H

#include "Strategy.h"
#include "MACrossBOLLTech.h"
#include "TechVec.h"

struct BOLLTech;
class IPositionControl;
class MACrossBOLLStrategy : public Strategy
{
public:
	MACrossBOLLStrategy(size_t short_ma, size_t long_ma, size_t boll_period, IPositionControl* accountMgr);
	virtual ~MACrossBOLLStrategy();

	virtual bool tryInvoke(const std::list<TickWrapper>& data, TickWrapper& info);
	virtual bool tryInvoke(const std::list<TickWrapper>& tickdata, const std::vector<KData>& data, std::vector<TickWrapper> curmindata, TickWrapper& info);
	virtual bool generateOrder(Order& out);
	virtual IPositionControl* getAccountMgr();

protected:
	virtual BOLLTech calculateBoll(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds) const;
	virtual BOLLTech calculateBoll(const std::vector<KData>& data, const KData& current, int mins) const;
	virtual double calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds) const;
	virtual double calculateK(const std::vector<KData>& data, const KData& current, int mins) const;
	//virtual MACrossBOLLTech* generateTechVec(const TickWrapper& info) const;

protected:
	size_t m_shortMA;
	size_t m_longMA;
	size_t m_bollperiod;

private:
	Order* m_curOrder;
	IPositionControl* m_AccoutMgr;

};

#endif