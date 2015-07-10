#pragma once
#include <list>

class TickWrapper;

class TechUtils
{
public:
	TechUtils();
	~TechUtils();

	static double CalulateMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds);
	static double CalulateEMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds);
	static double CalulateWMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds);
	static double CalulateAMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds);
};

