#ifndef POSITION_MGR_H
#define POSITION_MGR_H

#include <vector>
#include "ThostFtdcUserApiStruct.h"

namespace Position{

	enum PositionDirection{
		Buy = '0',
		Sell = '1',
	};

	class PositionMgr
	{
	public:
		static PositionMgr& Instance(){
			return m_instance;
		}
	
		~PositionMgr();

		void PushTradeItem(const CThostFtdcTradeField& item);

		/*the return value is position volume, it's a ratio from 0.0 ~ 1.0.
		  To support linked expression, also return position volume as double
		*/
		double GetPosition(double& pos, PositionDirection& direction);


	private:
		PositionMgr();
		PositionMgr(const PositionMgr& mgr) = delete;
		PositionMgr& operator = (const PositionMgr& mgr) = delete;

	private:
		static PositionMgr m_instance;
		std::vector<CThostFtdcTradeField> m_PositionList;
	};

};
#endif