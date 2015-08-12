#ifndef STRATEGY_H
#define STRATEGY_H

#include <list>
#include <vector>

class Order;
class TickWrapper;
class KData;
class IAccount;

class Strategy
{
public:
	Strategy(){}
	~Strategy(){}

	virtual bool tryInvoke(const std::list<TickWrapper>& data, TickWrapper& info) = 0;
	virtual bool tryInvoke(const std::list<TickWrapper>& tickdata, const std::vector<KData>& data, std::vector<TickWrapper> curmindata, TickWrapper& info) = 0;

	virtual bool generateOrder(Order& out) = 0;
	virtual IAccount* getAccountMgr() = 0;
};

struct StrategyMetaData{
	std::string name;
	size_t short_ma;
	size_t long_ma;
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
		AMA = 0X6,
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
		case MACrossStratgyType::AMA:
			return "AMA";
		default:
			return "Unknown";
		}
	};
};

typedef StratgyType::MACrossStratgyType CrossStratgyType;

#endif