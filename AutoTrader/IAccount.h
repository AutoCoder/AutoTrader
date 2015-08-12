#ifndef IACCOUNT_H
#define IACCOUNT_H

struct CThostFtdcTradingAccountField;
class Order;
class IAccount
{
public:
	IAccount(){};
	virtual ~IAccount(){};

	virtual void update(const CThostFtdcTradingAccountField& info) = 0;

	//return fail or success
	virtual bool completeOrder(Order& ord) = 0;

	virtual void setUpdated(bool val) = 0;
};

#endif