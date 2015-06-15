#ifndef AMACROSS_STRATEGY_H
#define AMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class AMACrossStratgy : public MACrossStratgy
{
public:
	AMACrossStratgy();
	virtual ~AMACrossStratgy();

protected:
	virtual double calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const;
	virtual MACrossStratgyTechVec* generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const;
};

#endif