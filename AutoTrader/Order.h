#pragma once
#include <string>

enum class ExchangeDirection{
	Buy = 0x0,
	Sell = 0x1,
};

class Order
{
public:
	Order();
	Order(const std::string& instrument, double refprice, ExchangeDirection direction);
	~Order();

	std::string GetInstrumentId() const{
		return m_instrumentId;
	}

private:

	enum class OrderType{
		LimitPrice = 0x0, 
		AnyPrice = 0x1, //Current Market Price
	};

	std::string m_instrumentId;
	double m_refExchangePrice;
	ExchangeDirection m_direction;
};

