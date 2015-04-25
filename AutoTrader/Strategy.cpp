#include "stdafx.h"
#include "Strategy.h"


Strategy::Strategy()
{
}


Strategy::~Strategy()
{
}

k3UpThroughK5::k3UpThroughK5()
{
}

k3UpThroughK5::~k3UpThroughK5()
{
}

bool k3UpThroughK5::check(const std::queue<CThostFtdcDepthMDFieldWrapper>& data)
{
	//fake implementation;
	int rd = rand();
	if (rd & 0x1)
		return true;
	else
		return false;
}
 
Order k3UpThroughK5::generateOrder(){
	return Order();
}