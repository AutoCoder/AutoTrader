#ifndef AMACROSS_STRATEGY_H
#define AMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class IAccount;
class AMACrossStratgy : public MACrossStratgy
{
public:
	AMACrossStratgy(size_t short_ma, size_t long_ma, IAccount* accountMgr);
	virtual ~AMACrossStratgy();

protected:
	virtual double calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds) const;
	virtual MACrossTech* generateTechVec(const TickWrapper& info) const;
};

#endif