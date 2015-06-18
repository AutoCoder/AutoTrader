#include "stdafx.h"
#include "AMACrossStratgy.h"
#include "ThostFtdcDepthMDFieldWrapper.h"

AMACrossStratgy::AMACrossStratgy(size_t short_ma, size_t long_ma)
:MACrossStratgy(short_ma, long_ma)
{
}


AMACrossStratgy::~AMACrossStratgy()
{
}

MACrossStratgyTechVec* AMACrossStratgy::generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const{
	return (new MACrossStratgyTechVec(CrossStratgyType::AMA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}

// AMA
/*
TurnOver == t
Volume == v
y=ama(x,a)£¬y=(t0 + t1 + t2 + t3 .. + tn)/(v0 + v1 + v2 + ...vn)
*/
double AMACrossStratgy::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
{
	//datetime to timestamp
	double totalExchangePrice = current.TurnOver();
	long long totalVolume = current.Volume();

	long long leftedge = current.toTimeStamp() - seconds * 2;
	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (it->toTimeStamp() > leftedge){
			totalExchangePrice += it->TurnOver();
			totalVolume += it->Volume();
		}
		else{
			break;
		}
	}

	//assert(totalVolume != 0);
	//assert(totalExchangePrice >= 0.0);

	return totalExchangePrice / totalVolume;
}
