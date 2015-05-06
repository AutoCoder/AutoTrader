#include "stdafx.h"
#include "Strategy.h"
#include <iostream>
const double minDelta = 0.3;

Strategy::Strategy()
{
}


Strategy::~Strategy()
{
}

double Strategy::calculateK(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds) const
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

	assert(totalVolume != 0);
	assert(totalExchangePrice >= 0.0);

	return totalExchangePrice / totalVolume;
}


k3UpThroughK5::k3UpThroughK5()
{
}

k3UpThroughK5::~k3UpThroughK5()
{
}

bool k3UpThroughK5::TryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info)
{
	bool orderSingal = false;
	double k3 = calculateK(data, info, 3 * 60);
	double k5 = calculateK(data, info, 5 * 60);
	info.setK3(k3);
	info.setK5(k5);

	//assert(!data.empty());
	if (data.empty())
		return false;

	auto preNode = data.begin();

	if (preNode->K5() > preNode->K3())
	{
		if (info.K3() > info.K5()){
			// Buy Singal
			// construct Buy Order ptr
			std::cout << "[Buy Signal]" << std::endl;
			std::cout << "LastPrice: " << info.LastPrice() << std::endl;
			orderSingal = true;	
		}
	}
	else{
		if (info.K3() < info.K5()){
			//Sell Singal
			// construct Sell Order ptr
			std::cout << "[Sell Signal]" << std::endl;
			std::cout << "LastPrice: " << info.LastPrice() << std::endl;
			orderSingal = true;
		}
	}

	return orderSingal;
}
 
Order k3UpThroughK5::generateOrder(){
	return Order();
}