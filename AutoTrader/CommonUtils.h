#pragma once

#include <time.h>
#include <string>


typedef int seconds;

class CommonUtils
{
public:
	CommonUtils();
	~CommonUtils();
	static seconds FromString(const char* time_src);

	// |bopen = false & eopen = true| -> [begin, end)
	// input_time belongTo [begin, end)
	static bool TimeInRange(const char* begin, bool bopen, const char* end, bool eopen,  const char* input);
	static bool IsMarketingTime(const char * time);

	static int StringtoInt(const std::string& str);
};

