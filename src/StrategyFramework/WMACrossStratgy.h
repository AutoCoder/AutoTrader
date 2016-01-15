#ifndef WMACROSS_STRATEGY_H
#define WMACROSS_STRATEGY_H

#include "stdafx.h"
#include "MACrossStratgy.h"

class IPositionControl;
class STRATEGY_API WMACrossStratgy : public MACrossStratgy
{
public:
	WMACrossStratgy(size_t short_ma, size_t long_ma, IPositionControl* accountMgr);
	virtual ~WMACrossStratgy();
	 
protected:
	virtual double calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds) const;
	virtual MACrossTech* generateTechVec(const TickWrapper& info) const;
};

#endif