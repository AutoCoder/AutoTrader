#ifndef ORDER_H
#define ORDER_H
#include <string>
#include "ThostFtdcUserApiStruct.h"

enum ExchangeDirection{
	Buy = '0',
	Sell = '1',
};

enum ExchangePriceType{
	AnyPrice = 0x0,
	LimitPrice = 0x1,
	LastPrice = 0x2,
};

enum TimeCondition{
	///立即完成，否则撤销
	IOC = '1',
	///本节有效
	GFS = '2',
	///当日有效
    GFD = '3',
	///指定日期前有效
	GTD = '4',
	///撤销前有效
	GTC = '5',
	///集合竞价有效
	GFA = '6',
};

enum VolumeCondition{
	///任何数量
	AnyVolume = '1',
	///最小数量
	MinVolume = '2',
	///全部数量
	WholeVolume = '3',
};

enum ContingentCondition{
	Immediately = '1',
	///止损
	Touch = '2',
	///止赢
	TouchProfit = '3',
	///预埋单
	ParkedOrder = '4',
	///最新价大于条件价
	LastPriceGreaterThanStopPrice = '5',
	///最新价大于等于条件价
	LastPriceGreaterEqualStopPrice = '6',
	///最新价小于条件价
	LastPriceLesserThanStopPrice = '7',
	///最新价小于等于条件价
	LastPriceLesserEqualStopPrice = '8',
	///卖一价大于条件价
	AskPriceGreaterThanStopPrice = '9',
	///卖一价大于等于条件价
	AskPriceGreaterEqualStopPrice = 'A',
	///卖一价小于条件价
	AskPriceLesserThanStopPrice = 'B',
	///卖一价小于等于条件价
	AskPriceLesserEqualStopPrice = 'C',
	///买一价大于条件价
	BidPriceGreaterThanStopPrice = 'D',
	///买一价大于等于条件价
	BidPriceGreaterEqualStopPrice = 'E',
	///买一价小于条件价
	BidPriceLesserThanStopPrice = 'F',
	///买一价小于等于条件价
	BidPriceLesserEqualStopPrice = 'H',
};



class Order
{
public:
	enum OrderType{
		LimitPriceFOKOrder = 0x0, //FAK: trade total successfully or total failed ; 
		LimitPriceFAKOrder = 0x1, //FOK: trade partly successfully while the others backout;
		AnyPriceOrder = 0x2,
		TriggerOrder = 0x3,
	};

	Order();
	//Order(const std::string& instrument, double refprice, \
	//	ExchangeDirection direction, \
	//	ExchangePriceType priceType, \
	//	TimeCondition timeCondition, \
	//	VolumeCondition vCondition, \
	//	ContingentCondition ctCondition);
	//Order(const std::string& instrument, double refprice, ExchangeDirection direction, OrderType type);
	~Order();

	std::string GetInstrumentId() const{
		return m_innerStruct.InstrumentID;
	}

	void SetInstrumentId(const std::string& in){
		strcpy_s(m_innerStruct.InstrumentID, sizeof(m_innerStruct.InstrumentID), in.c_str());
	}

	void SetRefExchangePrice(double price);

	double GetRefExchangePrice() const{
		//todo : distinguish the m_type
		return m_innerStruct.LimitPrice;
	}

	ExchangeDirection GetExchangeDirection() const {
		return (ExchangeDirection)m_innerStruct.Direction;
	}

	void SetExchangeDirection(ExchangeDirection in){
		m_innerStruct.Direction = in;
	}

	void SetCombOffsetFlagType(char flag){
		// todo: now only set the first byte
		m_innerStruct.CombOffsetFlag[0] = flag;
		for (int i = 1; i < 5; i++){
			m_innerStruct.CombOffsetFlag[i] = 0;
		}
	}

	void SetCombHedgeFlag(char flag){
		m_innerStruct.CombHedgeFlag[0] = flag;
		for (int i = 1; i < 5; i++){
			m_innerStruct.CombHedgeFlag[i] = 0;
		}
	}

	char GetCombOffsetFlagType() const {
		return m_innerStruct.CombOffsetFlag[0];
	}

	void SetVolume(int shou){
		m_innerStruct.VolumeTotalOriginal = shou;
	}

	int GetVolume() const{
		return m_innerStruct.VolumeTotalOriginal;
	}

	void SetOrderType(OrderType type);

	OrderType GetOrderType() const {
		return m_type;
	}

	void SetIdentityInfo(const std::string& brokerId, const std::string& userId, const std::string& investorId, const std::string& ordRef);

	bool IsValid();

	bool GetOrderOriginStruct(CThostFtdcInputOrderField& ord){
		if (IsValid()){
			ord = m_innerStruct;
			return true;
		}
		return false;
	}
private:
	CThostFtdcInputOrderField m_innerStruct;
	OrderType m_type;
};

#endif