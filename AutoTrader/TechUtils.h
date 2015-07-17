#pragma once
#include <list>
#include <vector>

class TickWrapper;
class KData;

class TechUtils
{
public:
	TechUtils();
	~TechUtils();

	static double CalulateMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds);
	static double CalulateEMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds);
	static double CalulateWMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds);
	static double CalulateAMA(const std::list<TickWrapper>& data, const TickWrapper& current, int seconds);

	static double CalulateMA(const std::vector<KData>& data, const KData& current, int mins);
	static double CalulateEMA(const std::vector<KData>& data, const KData& current, int mins);
	static double CalulateWMA(const std::vector<KData>& data, const KData& current, int mins);
	static double CalulateAMA(const std::vector<KData>& data, const KData& current, int mins);
};

