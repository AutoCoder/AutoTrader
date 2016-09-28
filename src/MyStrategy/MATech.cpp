#include "MATech.h"

#define MIN_DETLA 0.01

bool MATech::MAShortUpLong() const {
	return MAShortEqualLong() == false && mShortMAVal > mLongMAVal;
}

bool MATech::MAShortDownLong() const
{
	return MAShortEqualLong() == false && mShortMAVal < mLongMAVal;
}

bool MATech::MAShortEqualLong() const
{
	return std::fbs(mShortMAVal - mLongMAVal) < MIN_DETLA;
}