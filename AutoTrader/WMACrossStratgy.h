#ifndef WMACROSS_STRATEGY_H
#define WMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class IPositionControl;
class WMACrossStratgy : public MACrossStratgy
{
public:
	WMACrossStratgy(size_t short_ma, size_t long_ma, IPositionControl* accountMgr);
	virtual ~WMACrossStratgy();
	 
protected:
	virtual double calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds) const;
	virtual MACrossTech* generateTechVec(const TickWrapper& info) const;
};

#endif