#include "stdafx.h"
#include "Order.h"


Order::Order()
{
}


Order::~Order()
{
}


Order::Order(const std::string& instrument, double refprice, ExchangeDirection direction)
: m_instrumentId(instrument)
, m_refExchangePrice(refprice)
, m_direction(direction)
{

}