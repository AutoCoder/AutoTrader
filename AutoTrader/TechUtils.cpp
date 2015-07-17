#include "stdafx.h"
#include "TechUtils.h"
#include "TickWrapper.h"
#include "MACrossStratgy.h"
#include "KData.h"
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


double TechUtils::CalulateMA(const std::vector<KData>& data, const KData& current, int mins){
	//datetime to timestamp
	double totalExchangeLastPrice = current.LastPrice();
	long long count = 1;

	long long leftedge = current.Timestamp() - mins * 60 - 1;
	for (auto it = data.rbegin(); it != data.rend(); it++)
	{
		if (it->Timestamp() > leftedge){
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

double TechUtils::CalulateEMA(const std::vector<KData>& data, const KData& current, int mins){
	return 0.0;
	//if (data.empty()){
	//	return current.LastPrice();
	//}

	//TickWrapper preNode = data.back();
	//MACrossTech* preTechVec = dynamic_cast<MACrossTech*>(preNode.m_techvec);
	//if (preTechVec){
	//	if (mins == preTechVec->ShortMA()){
	//		double ret = (2 * current.LastPrice() + (N - 1)* preTechVec->ShortMA()) / (N + 1);
	//		return ret;
	//	}
	//	else if (mins == preTechVec->LongMA()){
	//		double ret = (2 * current.LastPrice() + (N - 1)* preTechVec->LongMA()) / (N + 1);
	//		return ret;
	//	}
	//	else{
	//		assert(false);
	//		return -1;
	//	}
	//}
	//else{
	//	return current.LastPrice();
	//}
}

double TechUtils::CalulateWMA(const std::vector<KData>& data, const KData& current, int mins){

	double totalExchangeLastPrice = current.LastPrice() * mins;
	long long count = mins--;

	long long leftedge = current.Timestamp() - mins * 60 - 1;

	for (auto it = data.rbegin(); it != data.rend(); it++)
	{
		if (it->Timestamp() > leftedge){
			totalExchangeLastPrice += (it->LastPrice() * mins);
			--mins;
			count += mins;
		}
		else{
			break;
		}
	}

	//assert(totalVolume != 0);
	//assert(totalExchangePrice >= 0.0);

	return totalExchangeLastPrice / count;
}

double TechUtils::CalulateAMA(const std::vector<KData>& data, const KData& current, int mins){
	double totalExchangePrice = current.TurnOver();
	long long totalVolume = current.Volume();

	long long leftedge = current.Timestamp() - mins * 60 - 1;
	for (auto it = data.rbegin(); it != data.rend(); it++)
	{
		if (it->Timestamp() > leftedge){
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