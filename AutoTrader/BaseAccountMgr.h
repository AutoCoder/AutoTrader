#ifndef BASE_ACCOUNT_H
#define BASE_ACCOUNT_H

#include "ThostFtdcUserApiStruct.h"
#include "IAccount.h"

class Order;
class BaseAccountMgr : public IAccount
{
public:
	BaseAccountMgr();
	virtual ~BaseAccountMgr();

	virtual void update(const CThostFtdcTradingAccountField& info);

	//return fail or success
	virtual bool completeOrder(Order& ord);

	
	virtual void setUpdated(bool val) { m_isUpdated = val;  }

private:
	CThostFtdcTradingAccountField m_accountInfo;
	bool m_isUpdated;
};

#endif