#include "stdafx.h"
#include "PositionMgr.h"

namespace Position{

	PositionMgr PositionMgr::m_instance;

	PositionMgr::PositionMgr()
	{
	}

	PositionMgr::~PositionMgr()
	{
		m_PositionList.clear();
	}

	void PositionMgr::PushTradeItem(const CThostFtdcTradeField& item){
		m_PositionList.push_back(item);
	}

	double PositionMgr::GetPosition(double& pos, PositionDirection& direction){
		pos = 0.1;
		direction = Buy;
		return pos;
	}

}