#ifndef Position_1_SHOU_H
#define Position_1_SHOU_H
#include "stdafx.h"

class Order;
namespace AP{
	class AccountDetailMgr;
	
};

class  Pos1Shou
{
public:
	explicit Pos1Shou();
	~Pos1Shou(){};
		
	void BindAccount(AP::AccountDetailMgr* mgr) { m_detailMgr = mgr; }
		
		//return fail or success
	virtual bool completeOrder(Order& ord);
		
private:
	AP::AccountDetailMgr* m_detailMgr;
				
};

#endif