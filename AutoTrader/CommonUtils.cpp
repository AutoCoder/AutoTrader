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

int CommonUtils::StringtoInt(const std::string& str)
{
	std::stringstream ss;
	int ret;
	ss << str;
	ss >> ret;
	return ret;
}

seconds CommonUtils::FromString(const char* time_src){
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
	seconds begin_s = FromString(begin);
	seconds end_s = FromString(end);
	seconds input_time_s = FromString(input_time);
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
		|| CommonUtils::TimeInRange("09:00:00", false, "11:30:00", true, time) \
		|| CommonUtils::TimeInRange("13:30:00", false, "15:00:00", true, time) \
		|| CommonUtils::TimeInRange("21:00:00", false, "23:59:59", false, time));

	return ret;
}