#ifndef WMACROSS_STRATEGY_H
#define WMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class WMACrossStratgy : MACrossStratgy
{
public:
	WMACrossStratgy();
	virtual ~WMACrossStratgy();
	 
protected:
	virtual double calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const;
	virtual MACrossStratgyTechVec* WMACrossStratgy::generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const;
};

#endif