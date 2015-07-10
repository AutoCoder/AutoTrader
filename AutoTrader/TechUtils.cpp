#include "stdafx.h"
#include "TechUtils.h"
#include "TickWrapper.h"
#include "MACrossStratgy.h"
#include <assert.h>

TechUtils::TechUtils()
{
}


TechUtils::~TechUtils()
{
}


double TechUtils::CalulateMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds)
{
	//datetime to timestamp
	double totalExchangeLastPrice = current.LastPrice();
	long long count = 1;

	long long leftedge = current.toTimeStamp() - seconds * 2;
	for (auto it = data.begin(); it != data.end(); it++)
	{
		if (it->toTimeStamp() > leftedge){
			totalExchangeLastPrice += it->LastPrice();
			++count;
		}
		else{
			break;
		}
	}

	//assert(totalVolume != 0);
	//assert(totalExchangePrice >= 0.0);

	return totalExchangeLastPrice / count;
}

double TechUtils::CalulateEMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds)
{
	if (data.empty()){
		return current.LastPrice();
	}

	int N = seconds * 2;

	TickWrapper preNode = data.front();
	MACrossTech* preTechVec = dynamic_cast<MACrossTech*>(preNode.m_techvec);
	if (preTechVec){
		if (seconds == 60 * preTechVec->ShortMA()){
			double ret = (2 * current.LastPrice() + (N - 1)* preTechVec->ShortMA()) / (N + 1);
			return ret;
		}
		else if (seconds == 60 * preTechVec->LongMA()){
			double ret = (2 * current.LastPrice() + (N - 1)* preTechVec->LongMA()) / (N + 1);
			return ret;
		}
		else{
			assert(false);
			return -1;
		}
	}
	else{
		return current.LastPrice();
	}
}

double TechUtils::CalulateWMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds)
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

double TechUtils::CalulateAMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds)
{
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
