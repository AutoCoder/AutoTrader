#include "stdafx.h"
#include "MACrossBOLLStrategy.h"
#include "MACrossBOLLTech.h"
#include "Order.h"
#include <assert.h>
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "BOLLTech.h"
#include "TechUtils.h"
#include <math.h>

MACrossBOLLStrategy::MACrossBOLLStrategy(size_t short_ma, size_t long_ma, size_t boll_period)
: m_shortMA(short_ma)
, m_longMA(long_ma)
, m_bollperiod(boll_period)
{
}


MACrossBOLLStrategy::~MACrossBOLLStrategy()
{
}


bool MACrossBOLLStrategy::tryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info){
	TickType direction = TickType::Commom;
	const size_t breakthrough_confirm_duration = 100; //50ms
	MACrossBOLLTech* curPtr = new MACrossBOLLTech(CrossStratgyType::MA, m_shortMA, m_longMA, info.UUID(), info.InstrumentId(), info.Time(), info.LastPrice());
	bool orderSingal = false;
	double short_ma = calculateK(data, info, m_shortMA * 60);
	double long_ma = calculateK(data, info, m_longMA * 60);
	BOLLTech bolltech = calculateBoll(data, info, m_shortMA * 60);
	curPtr->setShortMA(short_ma);
	curPtr->setLongMA(long_ma);
	curPtr->setBollTech(bolltech);
	return false;
}

Order MACrossBOLLStrategy::generateOrder(){
	assert(m_curOrder);
	return *m_curOrder;
}

BOLLTech MACrossBOLLStrategy::calculateBoll(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds, double ma_val) const{
	double ma = TechUtils::CalulateMA(data, current, seconds);
	int tickCount = 2 * seconds;

	double total = 0;
	long long count = 0;

	long long leftedge = current.toTimeStamp() - tickCount;
	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (it->toTimeStamp() > leftedge){
			double delta = it->LastPrice() - ma;
			total += (delta * delta);
			++count;
		}
		else{
			break;
		}
	}
	double var = sqrt(total / count);

	return BOLLTech(ma, var);
}


double MACrossBOLLStrategy::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const{
	return TechUtils::CalulateMA(data, current, seconds);
}