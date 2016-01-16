#ifndef IPositionControl_H
#define IPositionControl_H
#include "stdafx.h"

class Order;
namespace AP{
	class AccountDetailMgr;
	
};

class  Pos20Precent
{
public:
	explicit Pos20Precent();
	~Pos20Precent(){};
		
	void BindAccount(AP::AccountDetailMgr* mgr) { m_detailMgr = mgr; }
		
		//return fail or success
	virtual bool completeOrder(Order& ord);
		
private:
	AP::AccountDetailMgr* m_detailMgr;
				
};

#endif