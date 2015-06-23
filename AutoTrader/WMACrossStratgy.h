#ifndef WMACROSS_STRATEGY_H
#define WMACROSS_STRATEGY_H

#include "MACrossStratgy.h"

class WMACrossStratgy : public MACrossStratgy
{
public:
	WMACrossStratgy(size_t short_ma, size_t long_ma);
	virtual ~WMACrossStratgy();
	 
protected:
	virtual double calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const;
	virtual MACrossTech* generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const;
};

#endif