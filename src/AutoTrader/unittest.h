#ifndef CTP_UNITTEST_H
#define CTP_UNITTEST_H

#include <assert.h>
#include "CommonUtils.h"

void TestIsMarketingTime(){
	//[00:00:00, 01:00:00) & [09:00:00, 11:30:00) & [13:30:00, 15:00:00) & [21:00:00, 24:00:00]
	const char* t1 = "00:00:00"; assert(true==CommonUtils::IsMarketingTime(t1));
	const char* t2 = "01:00:00"; assert(false==CommonUtils::IsMarketingTime(t2));
	const char* t3 = "01:00:01"; assert(false==CommonUtils::IsMarketingTime(t3));
	const char* t4 = "05:00:01"; assert(false==CommonUtils::IsMarketingTime(t4));
	const char* t5 = "08:59:59"; assert(false==CommonUtils::IsMarketingTime(t5));
	const char* t6 = "09:00:00"; assert(true==CommonUtils::IsMarketingTime(t6));
	const char* t7 = "09:00:01"; assert(true==CommonUtils::IsMarketingTime(t7));
	const char* t8 = "11:29:59"; assert(true==CommonUtils::IsMarketingTime(t8));
	const char* t9 = "11:30:00"; assert(false==CommonUtils::IsMarketingTime(t9));
	const char* t11 = "11:30:01"; assert(false==CommonUtils::IsMarketingTime(t11));
	const char* t12 = "13:30:00"; assert(true==CommonUtils::IsMarketingTime(t12));
	const char* t13 = "13:29:59"; assert(false==CommonUtils::IsMarketingTime(t13));
	const char* t14 = "15:00:00"; assert(false==CommonUtils::IsMarketingTime(t14));
	const char* t15 = "15:00:01"; assert(false==CommonUtils::IsMarketingTime(t15));
	const char* t16 = "21:00:00"; assert(true==CommonUtils::IsMarketingTime(t16));
	const char* t17 = "23:59:59"; assert(true==CommonUtils::IsMarketingTime(t17));
}

void TestCreateVecTable(){
	//k3UpThroughK5TechVec vec1(123, "", "", 2333);
	//vec1.CreateTableIfNotExists("qihuo", "123543535");
}

void RunUnitTest(){
	TestIsMarketingTime();
}





#endif