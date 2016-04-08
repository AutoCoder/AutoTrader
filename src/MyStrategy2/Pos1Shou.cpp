#include "Pos1Shou.h"
#include "Order.h"
#include "AP_Mgr.h"


Pos1Shou::Pos1Shou()
{

}

bool Pos1Shou::completeOrder(Order& ord){
	if (!m_detailMgr)
		return false;

	double posMoney = 0.0;
	double available = 0.0;

	AP::Direction posDirection = AP::Long;
	m_detailMgr->getPosition(posMoney, posDirection, available);

	if (posMoney < std::numeric_limits<double>::epsilon()){
		ord.SetVolume(1);
		return true;
	}
	else{
		return false;
	}
}
