#ifndef Position_1_SHOU_H
#define Position_1_SHOU_H
#include "stdafx.h"
#include <vector>

class Order;
typedef std::vector<Order> OrderVec;

namespace PP{
	class PositionProfitMgr;
}

class  Pos1Shou
{
public:
	explicit Pos1Shou(int quantity = 1);
	~Pos1Shou(){};
		
	void BindAccount(PP::PositionProfitMgr* mgr) { m_ppMgr = mgr; }
		
		//return fail or success
	bool CompleteOrders(OrderVec& orders);
		
private:
	PP::PositionProfitMgr* m_ppMgr;
	int                   m_quantity;
};

#endif