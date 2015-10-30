#ifndef EMACROSS_STRATEGY_H
#define EMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class IPositionControl;
class EMACrossStratgy : public MACrossStratgy
{
public:
	EMACrossStratgy(size_t short_ma, size_t long_ma, IPositionControl* accountMgr);
	virtual ~EMACrossStratgy();

protected:
	virtual double calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds) const;
	virtual MACrossTech* generateTechVec(const TickWrapper& info) const;
};

#endif