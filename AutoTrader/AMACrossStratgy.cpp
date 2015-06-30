#include "stdafx.h"
#include "AMACrossStratgy.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "TechUtils.h"

AMACrossStratgy::AMACrossStratgy(size_t short_ma, size_t long_ma)
:MACrossStratgy(short_ma, long_ma)
{
}


AMACrossStratgy::~AMACrossStratgy()
{
}

MACrossTech* AMACrossStratgy::generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const{
	return (new MACrossTech(CrossStratgyType::AMA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}

// AMA
/*
TurnOver == t
Volume == v
y=ama(x,a)£¬y=(t0 + t1 + t2 + t3 .. + tn)/(v0 + v1 + v2 + ...vn)
*/
double AMACrossStratgy::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
{
	return TechUtils::CalulateAMA(data, current, seconds);
}
