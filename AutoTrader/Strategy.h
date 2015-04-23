#pragma once
#include "Order.h"

class Strategy
{
public:
	Strategy();
	~Strategy();

	virtual bool check() = 0;

	virtual Order generateOrder() = 0;
};

