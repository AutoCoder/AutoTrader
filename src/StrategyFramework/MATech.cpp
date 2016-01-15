#include "MATech.h"


bool MATech::IsTriggerPoint() const {
	return mShortMAVal > mLongMAVal;
}
