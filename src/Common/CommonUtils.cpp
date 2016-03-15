#include <cstdlib>
#include "CommonUtils.h"
#include <assert.h>
#include <sstream>
#include <cstring>
#include "ThostFtdcUserApiStruct.h"

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

bool CommonUtils::TimeInRange(const char* begin, bool bopen, const char* end, bool eopen, seconds input_time_s){
	seconds begin_s = TimeToSenconds(begin);
	seconds end_s = TimeToSenconds(end);

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

bool CommonUtils::TimeInRange(const char* begin, bool bopen, const char* end, bool eopen, const char* input_time){
	return TimeInRange(begin, bopen, end, eopen, TimeToSenconds(input_time));
}

bool CommonUtils::IsMarketingTime(seconds time){
	bool ret = (CommonUtils::TimeInRange("00:00:00", false, "01:00:00", true, time) \
		|| CommonUtils::TimeInRange("08:55:00", false, "11:30:00", true, time) \
		|| CommonUtils::TimeInRange("12:55:00", false, "15:00:00", true, time) \
		|| CommonUtils::TimeInRange("20:55:00", false, "23:59:59", false, time));

	return ret;
}

bool CommonUtils::IsMarketingTime(const char * time){
	// time belongTo [00:00:00, 01:00:00) & [09:00:00, 11:30:00) & [13:30:00, 15:00:00) & [21:00:00, 24:00:00]

	bool ret = (CommonUtils::TimeInRange("00:00:00", false, "01:00:00", true, time) \
		|| CommonUtils::TimeInRange("08:55:00", false, "11:30:00", true, time) \
		|| CommonUtils::TimeInRange("12:55:00", false, "15:00:00", true, time) \
		|| CommonUtils::TimeInRange("20:55:00", false, "23:59:59", false, time));

	return ret;
}

std::string CommonUtils::InterpretOrderStatusCode(TThostFtdcOrderStatusType type)
{
	switch (type){
	case THOST_FTDC_OST_AllTraded:
		return "Order is totally traded.";
	case THOST_FTDC_OST_PartTradedQueueing:
		return "Order is partly traded. Other is in queue.";
	case THOST_FTDC_OST_PartTradedNotQueueing:
		return "Order is partly traded. Other is abandoned.";
	case THOST_FTDC_OST_NoTradeQueueing:
		return "Order is not traded and is in queue.";
	case THOST_FTDC_OST_NoTradeNotQueueing:
		return "Order is not traded and is not in queue.";
	case THOST_FTDC_OST_Canceled:
		return "Order is cancelled.";
	case THOST_FTDC_OST_Unknown:
		return "Unknown status.";
	case THOST_FTDC_OST_NotTouched:
		return "Not triggered.";
	case THOST_FTDC_OST_Touched:
		return "triggered.";
	default:
		return "Wrong TThostFtdcOrderStatusType passed.";
	}
}

std::string CommonUtils::InterpretOrderSubmitStatusCode(TThostFtdcOrderSubmitStatusType type)
{
	switch (type){
	case THOST_FTDC_OSS_InsertSubmitted:
		return "Insert Order Submitted.";
	case THOST_FTDC_OSS_CancelSubmitted:
		return "Cancel Order Submitted.";
	case THOST_FTDC_OSS_ModifySubmitted:
		return "Modify Order Submitted.";
	case THOST_FTDC_OSS_Accepted:
		return "Order accepted.";
	case THOST_FTDC_OSS_InsertRejected:
		return "Order Insert rejected.";
	case THOST_FTDC_OSS_CancelRejected:
		return "Order Cancel rejected.";
	case THOST_FTDC_OSS_ModifyRejected:
		return "Order Modify rejected.";
	default:
		return "Wrong TThostFtdcOrderSubmitStatusType passed.";
	}
}

bool CommonUtils::InSameMinute(const std::string& time1, const std::string& time2){
	size_t len = time1.length();
	std::string nt1 = time1.substr(0, len - 2);
	std::string nt2 = time2.substr(0, len - 2);
	return nt1 == nt2;
}

std::string CommonUtils::StringFromStruct(const CThostFtdcInputOrderField& innerStruct){
	std::stringstream ss;

	ss << "BrokerID:" << innerStruct.BrokerID;
	ss << "\nInvestorID:" << innerStruct.InvestorID;
	ss << "\nInstrumentID:" << innerStruct.InstrumentID;
	ss << "\nOrderRef:" << innerStruct.OrderRef;
	ss << "\nUserID:" << innerStruct.UserID;
	ss << "\nOrderPriceType:" << innerStruct.OrderPriceType;
	ss << "\nDirection:" << innerStruct.Direction;
	ss << "\nCombOffsetFlag:" << innerStruct.CombOffsetFlag;
	ss << "\nCombHedgeFlag:" << innerStruct.CombHedgeFlag;
	ss << "\nLimitPrice:" << innerStruct.LimitPrice;
	ss << "\nVolumeTotalOriginal:" << innerStruct.VolumeTotalOriginal;
	ss << "\nTimeCondition:" << innerStruct.TimeCondition;
	ss << "\nGTDDate:" << innerStruct.GTDDate;
	ss << "\nVolumeCondition:" << innerStruct.VolumeCondition;
	ss << "\nMinVolume:" << innerStruct.MinVolume;
	ss << "\nContingentCondition:" << innerStruct.ContingentCondition;
	ss << "\nStopPrice:" << innerStruct.StopPrice;
	ss << "\nForceCloseReason:" << innerStruct.ForceCloseReason;
	ss << "\nIsAutoSuspend:" << innerStruct.IsAutoSuspend;
	ss << "\nBusinessUnit:" << innerStruct.BusinessUnit;
	ss << "\nRequestID:" << innerStruct.RequestID;
	ss << "\nUserForceClose:" << innerStruct.UserForceClose;
	ss << "\nIsSwapOrder:" << innerStruct.IsSwapOrder;
	return ss.str();
}

std::string CommonUtils::ConvertOrderListToString(const std::vector< CThostFtdcOrderField >& list){
	if (list.empty())
		return "";

	std::stringstream result;
	result << std::endl << "------------------------------------------------" << std::endl;

	for (auto iter = list.begin(); iter != list.end(); iter++){
		result << "经纪公司代码:" << iter->BrokerID << std::endl
			<< " 投资者代码:" << iter->InvestorID << std::endl
			<< " 用户代码:" << iter->UserID << std::endl
			<< " 合约代码:" << iter->InstrumentID << std::endl
			<< " 买卖方向:" << iter->Direction << std::endl
			<< " 组合开平标志:" << iter->CombOffsetFlag << std::endl
			<< " 价格:" << iter->LimitPrice << std::endl
			<< " 数量:" << iter->VolumeTotalOriginal << std::endl
			<< " 报单引用:" << iter->OrderRef << std::endl
			<< " 客户代码:" << iter->ClientID << std::endl
			<< " 报单状态:" << iter->OrderStatus << std::endl
			<< " 委托时间:" << iter->InsertTime << std::endl
			<< " 报单编号:" << iter->OrderSysID << std::endl
			<< " GTD日期:" << iter->GTDDate << std::endl
			<< " 交易日:" << iter->TradingDay << std::endl
			<< " 报单日期:" << iter->InsertDate << std::endl;
	}
	result << "--------------------------------------------------" << std::endl;
	return result.str();
}

std::string CommonUtils::ConvertTradeListToString(const std::vector< CThostFtdcTradeField >& list){
	if (list.empty())
		return "";

	std::stringstream result;
	result << std::endl << "-----------------------" << std::endl;

	for (auto iter = list.begin(); iter != list.end(); iter++){
		result << "合约代码:" << iter->InstrumentID << std::endl
			<< " 用户代码:" << iter->UserID << std::endl
			<< " 成交编号:" << iter->TradeID << std::endl
			<< " 买卖方向:" << iter->Direction << std::endl
			<< " 开平标志:" << iter->OffsetFlag << std::endl
			<< " 投机套保标志:" << iter->HedgeFlag << std::endl
			<< " 价格:" << iter->Price << std::endl
			<< " 数量:" << iter->Volume << std::endl
			<< " 成交时间:" << iter->TradeTime << std::endl
			<< " 成交类型:" << iter->TradeType << std::endl
			<< " 报单编号:" << iter->OrderSysID << std::endl
			<< " 报单引用:" << iter->OrderRef << std::endl
			<< " 本地报单编号:" << iter->OrderLocalID << std::endl
			<< " 业务单元:" << iter->BusinessUnit << std::endl
			<< " 序号:" << iter->SequenceNo << std::endl
			<< " 经纪公司报单编号:" << iter->BrokerOrderSeq << std::endl
			<< " 成交时期:" << iter->TradeDate << std::endl
			<< " 交易日:" << iter->TradingDay << std::endl;
	}
	result << "-----------------------" << std::endl;
	return result.str();
}
