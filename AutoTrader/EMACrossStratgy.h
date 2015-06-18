#ifndef EMACROSS_STRATEGY_H
#define EMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class EMACrossStratgy : public MACrossStratgy
{
public:
	EMACrossStratgy(size_t short_ma, size_t long_ma);
	virtual ~EMACrossStratgy();

protected:
	virtual double calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const;
	virtual MACrossStratgyTechVec* generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const;
};

#endif