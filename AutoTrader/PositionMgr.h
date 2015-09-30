#ifndef POSITION_MGR_H
#define POSITION_MGR_H

#include <vector>
#include <mutex>
#include "ThostFtdcUserApiStruct.h"

namespace Position{

	enum PositionDirection{
		Buy = '0',
		Sell = '1',
	};

	/*As the PositionMgr will be only called by tradespi thread, so don't supply concurrency support for now*/
	class PositionMgr
	{
	public:
		PositionMgr();
		~PositionMgr();

		void PushTradeItem(const CThostFtdcTradeField& item);

		/*the return value is position volume, it's a ratio from 0.0 ~ 1.0.
		  To support linked expression, also return position volume as double
		*/
		double GetPosition(double& pos, PositionDirection& direction, int& volume) const;

	private:
		PositionMgr(const PositionMgr& mgr) = delete;
		PositionMgr& operator = (const PositionMgr& mgr) = delete;

	private:
		mutable std::mutex m_mutex;
		std::vector<CThostFtdcTradeField> m_PositionList;
	};

	PositionMgr& GetManager();

};
#endif