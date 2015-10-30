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

	static double CalulateMA(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds);
	static double CalulateEMA(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds);
	static double CalulateWMA(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds);
	static double CalulateAMA(const std::list<TickWrapper>& data, const TickWrapper& current, size_t seconds);

	static double CalulateMA(const std::vector<KData>& data, const KData& current, size_t mins);
	static double CalulateEMA(const std::vector<KData>& data, const KData& current, size_t mins);
	static double CalulateWMA(const std::vector<KData>& data, const KData& current, size_t mins);
	static double CalulateAMA(const std::vector<KData>& data, const KData& current, size_t mins);
};

