//#include "stdafx.h"
#include "EMACrossStratgy.h"
#include "TickWrapper.h"
#include "TechUtils.h"

EMACrossStratgy::EMACrossStratgy(size_t short_ma, size_t long_ma)
:MACrossStratgy(short_ma, long_ma)
{
}


EMACrossStratgy::~EMACrossStratgy()
{
}

/*
EMA(X£¬1) = £Û2*X1 + (1 - 1)*Y¡¯£Ý / (1 + 1) = X1
EMA(X, N) = [ 2*X + (N-1)*Y'] / (N+1) 
*/
double EMACrossStratgy::calculateK(const std::vector<TickWrapper>& data, const TickWrapper& current, size_t seconds) const
{
	return TechUtils::CalulateEMA(data, current, seconds);
}

MACrossTech* EMACrossStratgy::generateTechVec(const TickWrapper& info) const
{
	return (new MACrossTech(CrossStratgyType::EMA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}