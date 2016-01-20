#include "BOLLTech.h"


bool BOLLTech::IsTriggerPoint() const{
	// if the cur_value is in [BollMd, BollUp], regards this point is BreakPoint
	return mCurValue > (mMAValue + m_boll_k * mVariance / 2);
}