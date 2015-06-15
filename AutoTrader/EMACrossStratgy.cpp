#include "stdafx.h"
#include "EMACrossStratgy.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include <assert.h>

EMACrossStratgy::EMACrossStratgy()
:MACrossStratgy()
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
		if (N == 60 * 3){
			double ret = (2 * current.LastPrice() + (N - 1)* preTechVec->K3m()) / (N + 1);
			return ret;
		}
		else if (N == 60 * 5){
			double ret = (2 * current.LastPrice() + (N - 1)* preTechVec->K5m()) / (N + 1);
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
	return (new MACrossStratgyTechVec(CrossStratgyType::EMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}