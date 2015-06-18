#include "stdafx.h"
#include "WMACrossStratgy.h"
#include "ThostFtdcDepthMDFieldWrapper.h"

WMACrossStratgy::WMACrossStratgy(size_t short_ma, size_t long_ma)
: MACrossStratgy(short_ma, long_ma)
{
}

WMACrossStratgy::~WMACrossStratgy()
{
}

MACrossStratgyTechVec* WMACrossStratgy::generateTechVec(const CThostFtdcDepthMDFieldWrapper& info) const{
	return (new MACrossStratgyTechVec(CrossStratgyType::WMA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice()));
}

// WMA
/*
y=wma(x,a)£¬y=(n*x0+(n-1)*x1+(n- 2)*x2)+...+1*xn)/(n+(n-1)+(n-2)+...+1)
*/
double WMACrossStratgy::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
{
	//datetime to timestamp
	int n = seconds * 2; 
	double totalExchangeLastPrice = current.LastPrice() * n;
	long long count = n--;

	long long leftedge = current.toTimeStamp() - seconds * 2;
	
	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (it->toTimeStamp() > leftedge){
			totalExchangeLastPrice += (it->LastPrice() * n);
			--n;
			count += n;
		}
		else{
			break;
		}
	}

	//assert(totalVolume != 0);
	//assert(totalExchangePrice >= 0.0);

	return totalExchangeLastPrice / count;
}
