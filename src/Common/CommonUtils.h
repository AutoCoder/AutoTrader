#ifndef COMMONUTILS_H
#define COMMONUTILS_H
#include "Common.h"

#include "ThostFtdcUserApiDataType.h"
#include <time.h>
#include <string>
#include <vector>

typedef int seconds;
struct CThostFtdcInputOrderField;
struct CThostFtdcOrderField;
struct CThostFtdcTradeField;

class COMMON_API CommonUtils
{
public:
	CommonUtils();
	~CommonUtils();
	static seconds TimeToSenconds(const char* time_src);
	static long long DateTimeToTimestamp(const char* date_src, const char* time_src);

	// |bopen = false & eopen = true| -> [begin, end)
	// input_time belongTo [begin, end)
	static bool TimeInRange(const char* begin, bool bopen, const char* end, bool eopen,  const char* input);
	static bool TimeInRange(const char* begin, bool bopen, const char* end, bool eopen, seconds input_time_s);
	static bool IsMarketingTime(const char * time);
	static bool IsMarketingTime(seconds input_time_s);

	static int StringtoInt(const std::string& str);
	static double StringtoDouble(const std::string& str);
	static long long Stringtolong(const std::string& str);
	static std::string ConvertTime(const std::string& src);

	static std::string InterpretOrderStatusCode(TThostFtdcOrderStatusType type);
	static std::string InterpretOrderSubmitStatusCode(TThostFtdcOrderSubmitStatusType type);

	static bool InSameMinute(const std::string& time1, const std::string& time2);
	static std::string StringFromStruct(const CThostFtdcInputOrderField& innerStruct);

	static std::string ConvertOrderToString(const CThostFtdcOrderField& list);
	static std::string ConvertOrderListToString(const std::vector< CThostFtdcOrderField >& list);
	static std::string ConvertTradeListToString(const std::vector< CThostFtdcTradeField >& list);

};

#endif