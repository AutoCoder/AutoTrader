#ifndef AMACROSS_STRATEGY_H
#define AMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class AMACrossStratgy : public MACrossStratgy
{
public:
	AMACrossStratgy(size_t short_ma, size_t long_ma);
	virtual ~AMACrossStratgy();

protected:
	virtual double calculateK(const std::vector<TickWrapper>& data, const TickWrapper& current, size_t seconds) const;
	virtual MACrossTech* generateTechVec(const TickWrapper& info) const;
};

#endif