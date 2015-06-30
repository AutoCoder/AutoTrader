#include "stdafx.h"
#include "EMACrossStratgy.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include <assert.h>
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
double EMACrossStratgy::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
{
	return TechUtils::CalulateEMA(data, current, seconds);
}

MACrossTech* EMACrossStratgy::generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const
{
	return (new MACrossTech(CrossStratgyType::EMA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}