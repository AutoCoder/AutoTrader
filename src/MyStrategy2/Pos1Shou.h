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
	explicit Pos1Shou(int quantity = 1);
	~Pos1Shou(){};
		
	void BindAccount(AP::AccountDetailMgr* mgr) { m_detailMgr = mgr; }
		
		//return fail or success
	bool CompleteOrder(OrderVec& orders);
		
private:
	AP::AccountDetailMgr* m_detailMgr;
	int                   m_quantity;
};

#endif