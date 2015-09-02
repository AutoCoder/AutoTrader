#ifndef BASE_ACCOUNT_H
#define BASE_ACCOUNT_H

#include "ThostFtdcUserApiStruct.h"
#include "IAccount.h"

class Order;
class BaseAccountMgr : public IAccount
{
public:
	BaseAccountMgr(TThostFtdcInstrumentIDType instr);
	virtual ~BaseAccountMgr();

	virtual void update(const CThostFtdcTradingAccountField& info);

	virtual void update(const CThostFtdcInvestorPositionField& info);

	//return fail or success
	virtual bool completeOrder(Order& ord);

	
	virtual void setUpdated(bool val) { m_isAccountUpdated = val; m_isPositionUpdated = val; }

	virtual bool isUpdated() { return m_isAccountUpdated && m_isPositionUpdated; }

	virtual char* InstrumentID() { return m_instrument; }
private:
	TThostFtdcInstrumentIDType m_instrument;
	CThostFtdcTradingAccountField m_accountInfo;
	CThostFtdcInvestorPositionField m_positionInfo;
	volatile bool m_isAccountUpdated;
	volatile bool m_isPositionUpdated;

};

#endif