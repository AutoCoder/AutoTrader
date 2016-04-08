#ifndef WMACROSS_STRATEGY_H
#define WMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class WMACrossStratgy : public MACrossStratgy
{
public:
	WMACrossStratgy(size_t short_ma, size_t long_ma);
	virtual ~WMACrossStratgy();
	 
protected:
	virtual double calculateK(const std::vector<TickWrapper>& data, const TickWrapper& current, size_t seconds) const;
	virtual MACrossTech* generateTechVec(const TickWrapper& info) const;
};

#endif