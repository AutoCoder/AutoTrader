#include "stdafx.h"
#include "EMACrossStratgy.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include <assert.h>

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
	if (data.empty()){
		return current.LastPrice();
	}

	int N = seconds * 2;

	CThostFtdcDepthMDFieldWrapper preNode = data.front();
	MACrossStratgyTechVec* preTechVec = dynamic_cast<MACrossStratgyTechVec*>(preNode.m_techvec);
	if (preTechVec){
		if (seconds == 60 * m_shortMA ){
			double ret = (2 * current.LastPrice() + (N - 1)* preTechVec->ShortMA()) / (N + 1);
			return ret;
		}
		else if (seconds == 60 * m_longMA){
			double ret = (2 * current.LastPrice() + (N - 1)* preTechVec->LongMA()) / (N + 1);
			return ret;
		}
		else{
			assert(false);
		}
	}
	else{
		return current.LastPrice();
	}
}

MACrossStratgyTechVec* EMACrossStratgy::generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const
{
	return (new MACrossStratgyTechVec(CrossStratgyType::EMA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}