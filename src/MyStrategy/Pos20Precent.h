#ifndef IPositionControl_H
#define IPositionControl_H
#include "stdafx.h"
#include <vector>

class Order;
typedef std::vector<Order> OrderVec;

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
	bool CompleteOrders(OrderVec& orders);
		
private:
	AP::AccountDetailMgr* m_detailMgr;
				
};

#endif