#ifndef STRATEGY_H
#define STRATEGY_H

#include <list>

class Order;
class CThostFtdcDepthMDFieldWrapper;

class Strategy
{
public:
	Strategy(){}
	~Strategy(){}

	virtual bool tryInvoke(const std::list<CThostFtdcDepthMDFieldWrapper>& data, CThostFtdcDepthMDFieldWrapper& info) = 0;

	virtual Order generateOrder() = 0;

};

class StratgyType{
public:
	enum class MACrossStratgyType{
		MA = 0x0,
		EMA = 0x1,
		SMA = 0x2,
		DMA = 0x3,
		TMA = 0x4,
		WMA = 0x5,
	};

	static std::string toString(MACrossStratgyType type){
		switch (type){
		case MACrossStratgyType::MA:
			return "MA";
		case MACrossStratgyType::EMA:
			return "EMA";
		case MACrossStratgyType::SMA:
			return "SMA";
		case MACrossStratgyType::DMA:
			return "DMA";
		case MACrossStratgyType::TMA:
			return "TMA";
		case MACrossStratgyType::WMA:
			return "WMA";
		default:
			return "Unknown";
		}
	};
};

#endif