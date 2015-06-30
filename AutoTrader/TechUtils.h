#pragma once
#include <list>

class CThostFtdcDepthMDFieldWrapper;

class TechUtils
{
public:
	TechUtils();
	~TechUtils();

	static double CalulateMA(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds);
	static double CalulateEMA(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds);
	static double CalulateWMA(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds);
	static double CalulateAMA(const std::list<CThostFtdcDepthMDFieldWrapper>& data, const CThostFtdcDepthMDFieldWrapper& current, int seconds);
};

