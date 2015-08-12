#ifndef EMACROSS_STRATEGY_H
#define EMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class IAccount;
class EMACrossStratgy : public MACrossStratgy
{
public:
	EMACrossStratgy(size_t short_ma, size_t long_ma, IAccount* accountMgr);
	virtual ~EMACrossStratgy();

protected:
	virtual double calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds) const;
	virtual MACrossTech* generateTechVec(const TickWrapper& info) const;
};

#endif