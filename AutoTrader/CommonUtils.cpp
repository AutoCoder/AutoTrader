#include "stdafx.h"
#include "CommonUtils.h"
#include <assert.h>
#include <sstream>
CommonUtils::CommonUtils()
{
}


CommonUtils::~CommonUtils()
{
}

/*
Usage: "20150430" to 1430382950
*/
long long CommonUtils::DateTimeToTimestamp(const char* date_src, const char* time_src){
	char szYear[5], szMonth[3], szDay[3], szHour[3], szMin[3], szSec[3];

	szYear[0] = *date_src++;
	szYear[1] = *date_src++;
	szYear[2] = *date_src++;
	szYear[3] = *date_src++;
	szYear[4] = 0x0;

	szMonth[0] = *date_src++;
	szMonth[1] = *date_src++;
	szMonth[2] = 0x0;

	szDay[0] = *date_src++;
	szDay[1] = *date_src++;
	szDay[2] = 0x0;

	szHour[0] = *time_src++;
	szHour[1] = *time_src++;
	szHour[2] = 0x0;
	time_src++;

	szMin[0] = *time_src++;
	szMin[1] = *time_src++;
	szMin[2] = 0x0;
	time_src++;

	szSec[0] = *time_src++;
	szSec[1] = *time_src++;
	szSec[2] = 0x0;

	tm tmObj;

	tmObj.tm_year = atoi(szYear) - 1900;
	tmObj.tm_mon = atoi(szMonth) - 1;
	tmObj.tm_mday = atoi(szDay);
	tmObj.tm_hour = atoi(szHour);
	tmObj.tm_min = atoi(szMin);
	tmObj.tm_sec = atoi(szSec);
	tmObj.tm_isdst = -1;

	return mktime(&tmObj);
}

double CommonUtils::StringtoDouble(const std::string& str)
{
	std::stringstream ss;
	double ret;
	ss << str;
	ss >> ret;
	return ret;
}

int CommonUtils::StringtoInt(const std::string& str)
{
	std::stringstream ss;
	int ret;
	ss << str;
	ss >> ret;
	return ret;
}

long long CommonUtils::Stringtolong(const std::string& str)
{
	std::stringstream ss;
	long long ret;
	ss << str;
	ss >> ret;
	return ret;
}

std::string CommonUtils::ConvertTime(const std::string& src){
	//from "2015-04-30 00:00:00" to "20150430"
	//assert(src.length() > 10);
	char ret[9];
	const char* _src = src.c_str();

	//year
	ret[0] = *_src++;
	ret[1] = *_src++;
	ret[2] = *_src++;
	ret[3] = *_src++;
	_src++;//skip '-'

	//month
	ret[4] = *_src++;
	ret[5] = *_src++;
	_src++;

	//day
	ret[6] = *_src++;
	ret[7] = *_src++;
	ret[8] = 0x0;

	return std::string(ret);
}

seconds CommonUtils::TimeToSenconds(const char* time_src){
	assert(strlen(time_src) == 8);

	char szHour[3], szMin[3], szSec[3];
	szHour[0] = *time_src++;
	szHour[1] = *time_src++;
	szHour[2] = 0x0;
	time_src++;

	szMin[0] = *time_src++;
	szMin[1] = *time_src++;
	szMin[2] = 0x0;
	time_src++;

	szSec[0] = *time_src++;
	szSec[1] = *time_src++;
	szSec[2] = 0x0;

	int hours = StringtoInt(szHour);
	if (hours > 23)
		throw("Hour can't bigger than 23");

	int minutes = StringtoInt(szMin);
	if (minutes > 59)
		throw("Minutes can't bigger than 59");

	int seconds = StringtoInt(szSec);
	if (seconds > 59)
		throw("Seconds can't bigger than 59");

	int ret = atoi(szHour) * 60 * 60 + atoi(szMin) * 60 + atoi(szSec);

	return ret;
}

bool CommonUtils::TimeInRange(const char* begin, bool bopen, const char* end, bool eopen, const char* input_time){
	seconds begin_s = TimeToSenconds(begin);
	seconds end_s = TimeToSenconds(end);
	seconds input_time_s = TimeToSenconds(input_time);
	bool leftIn = false;
	bool RightIn = false;

	if (bopen == true && begin_s < input_time_s)
		leftIn = true;
	else if (bopen == false && begin_s <= input_time_s)
		leftIn = true;

	if (leftIn == false)
		return false;

	if (eopen == true && input_time_s < end_s)
		RightIn = true;
	else if (eopen == false && input_time_s <= end_s)
		RightIn = true;

	return RightIn;
}

bool CommonUtils::IsMarketingTime(const char * time){
	// time belongTo [00:00:00, 01:00:00) & [09:00:00, 11:30:00) & [13:30:00, 15:00:00) & [21:00:00, 24:00:00]

	bool ret = (CommonUtils::TimeInRange("00:00:00", false, "01:00:00", true, time) \
		|| CommonUtils::TimeInRange("08:55:00", false, "11:30:00", true, time) \
		|| CommonUtils::TimeInRange("13:25:00", false, "15:00:00", true, time) \
		|| CommonUtils::TimeInRange("20:55:00", false, "23:59:59", false, time));

	return ret;
}