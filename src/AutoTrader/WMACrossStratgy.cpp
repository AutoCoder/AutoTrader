#include "stdafx.h"
#include "WMACrossStratgy.h"
#include "TickWrapper.h"
#include "TechUtils.h"

WMACrossStratgy::WMACrossStratgy(size_t short_ma, size_t long_ma, IPositionControl* accountMgr)
: MACrossStratgy(short_ma, long_ma, accountMgr)
{
}

WMACrossStratgy::~WMACrossStratgy()
{
}

MACrossTech* WMACrossStratgy::generateTechVec(const TickWrapper& info) const{
	return (new MACrossTech(CrossStratgyType::WMA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}

// WMA
/*
y=wma(x,a)£¬y=(n*x0+(n-1)*x1+(n- 2)*x2)+...+1*xn)/(n+(n-1)+(n-2)+...+1)
*/
double WMACrossStratgy::calculateK(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds) const
{
	return TechUtils::CalulateWMA(data, current, seconds);
}
