#include "stdafx.h"
#include "PositionMgr.h"
#include "InstrumentInfoMgr.h"
#include <assert.h>

namespace Position{

	PositionMgr& GetManager(){
		static PositionMgr mgr;
		return mgr;
	}

	PositionMgr::PositionMgr()
	{
	}

	PositionMgr::~PositionMgr()
	{
		m_PositionList.clear();
	}

	/* this function is called in tradespi thread*/
	void PositionMgr::PushTradeItem(const CThostFtdcTradeField& item){
		std::lock_guard<std::mutex> lk(m_mutex);
		m_PositionList.push_back(item);
	}


	/*
	This thread is called by in mdspi thread
	thought : is it needed to add synchronization for functions (PositionMgr::PushTradeItem, PositionMgr::GetPosition) ?
	Aspects:
	  (1) As these two functions will run in two threads. so it is needed to add lock.
	  (2) As "GetPosition" always want to get the newest poistion Info, so that if there are new Position change, it's better to give up current "GetPosition", and calculate again totally.
	*/
	double PositionMgr::GetPosition(double& pos, PositionDirection& direction, int &volume) const{
		double amount_buy = 0.0; //THOST_FTDC_D_Buy
		int volume_buy = 0;
		double amount_sell = 0.0; // THOST_FTDC_D_Sell
		int volume_sell = 0;

		std::unique_lock<std::mutex> lk(m_mutex);
		for (auto item : m_PositionList){

			Instrument::InformationMgr& posMgr = Instrument::GetManager();
			int multiple = posMgr.GetVolumeMultiple(item.InstrumentID);
			if (item.Direction == THOST_FTDC_D_Buy){
				amount_buy += (item.Price * item.Volume * multiple);
				volume_buy += item.Volume;
			}
			else if (item.Direction == THOST_FTDC_D_Sell){
				amount_sell += (item.Price * item.Volume * multiple);
				volume_sell += item.Volume;
			}
			else{
				assert(false);
			}
		}
		lk.unlock();
		direction = amount_buy > amount_sell ? Position::Buy : Position::Sell;
		pos = amount_buy > amount_sell ? (amount_buy - amount_sell) : (amount_sell - amount_buy);
		volume = volume_buy > volume_sell ? (volume_buy - volume_sell) : (volume_sell - volume_buy);
		return pos;
	}
}