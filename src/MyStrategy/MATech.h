#ifndef MA_TECH_H
#define MA_TECH_H

#include "stdafx.h"

struct MATech{
	MATech(){};
	~MATech(){};

	double mLongMAVal; 
	double mShortMAVal;

	bool IsTriggerPoint() const;
};

#endif