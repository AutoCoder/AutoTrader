#ifndef TEST_CANCEL_POS_MANAGE_H
#define TEST_CANCEL_POS_MANAGE_H
#include "stdafx.h"
#include <vector>

class Order;
typedef std::vector<Order> OrderVec;

namespace PP{
	class PositionProfitMgr;
}

class  TestCancelPosManage
{
public:
	explicit TestCancelPosManage();
	~TestCancelPosManage(){};
		
	void BindAccount(PP::PositionProfitMgr* mgr) { m_ppMgr = mgr; }
		
		//return fail or success
	bool CompleteOrders(OrderVec& orders);
		
private:
	PP::PositionProfitMgr* m_ppMgr;
};

#endif