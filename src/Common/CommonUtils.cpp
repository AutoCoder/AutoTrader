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
		return "AllTraded";
	case THOST_FTDC_OST_PartTradedQueueing:
		return "PartTradedQueueing";
	case THOST_FTDC_OST_PartTradedNotQueueing:
		return "PartTradedNotQueueing";// "Order is partly traded. Other is abandoned.";
	case THOST_FTDC_OST_NoTradeQueueing:
		return "NoTradeQueueing";// "Order is not traded and is in queue.";
	case THOST_FTDC_OST_NoTradeNotQueueing:
		return "NoTradeNotQueueing";// "Order is not traded and is not in queue.";
	case THOST_FTDC_OST_Canceled:
		return "Canceled";// "Order is cancelled.";
	case THOST_FTDC_OST_Unknown:
		return "Unknown";// "Unknown status.";
	case THOST_FTDC_OST_NotTouched:
		return "NotTouched";// "Not triggered.";
	case THOST_FTDC_OST_Touched:
		return "Touched";// "triggered.";
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

std::string CommonUtils::ConvertOrderToString(const CThostFtdcOrderField& order){
	std::stringstream result;
	result << "BrokerID:" << order.BrokerID << std::endl
	<< " InvestorID:" << order.InvestorID << std::endl
	<< " UserID:" << order.UserID << std::endl
	<< " InstrumentID:" << order.InstrumentID << std::endl
	<< " Direction:" << order.Direction << std::endl
	<< " CombOffsetFlag:" << order.CombOffsetFlag << std::endl
	<< " LimitPrice:" << order.LimitPrice << std::endl
	<< " VolumeTotalOriginal:" << order.VolumeTotalOriginal << std::endl
	<< " OrderRef:" << order.OrderRef << std::endl
	<< " ClientID:" << order.ClientID << std::endl
	<< " OrderStatus:" << InterpretOrderStatusCode(order.OrderStatus) << std::endl
	<< " BrokerOrderSeq:" << order.BrokerOrderSeq << std::endl
	<< " InsertTime:" << order.InsertTime << std::endl
	<< " OrderSysID:" << order.OrderSysID << std::endl
	<< " GTDDate:" << order.GTDDate << std::endl
	<< " TradingDay:" << order.TradingDay << std::endl
	<< " InsertDate:" << order.InsertDate << std::endl;
	return result.str();
}

std::string CommonUtils::OffsetFlagToString(char flag){	
	std::string tradeType = "";
	switch (flag)
	{
		case '0':
			tradeType = "Open";
			break;
		case '1':
			tradeType = "Close";
			break;
		case '2':
			tradeType = "ForceClose";
			break;
		case '3':
			tradeType = "CloseToday";
			break;
		case '4':
			tradeType = "CloseYesterday";
			break;
		case '5':
			tradeType = "ForceOff";
			break;
		case '6':
			tradeType = "LocalForceClose";
			break;
		default:
			tradeType = "Unknown";
			break;
	}

	return tradeType;
}

std::string CommonUtils::InstrumentIDToProductID(const std::string& instrumentID){
	assert(!instrumentID.empty());
	size_t max_idx = instrumentID.size();
	while (true){
		char c = instrumentID.at(max_idx-1);
		if (c >= '0' && c <= '9')
		{
			--max_idx;
		}
		else{
			break;
		}
	}

	
	return instrumentID.substr(0, max_idx);
}

std::string CommonUtils::ConvertTradeToString(const CThostFtdcTradeField& trade){
	std::stringstream result;
	
	result << "InstrumentID:" << trade.InstrumentID << std::endl
		<< " Direction:" << trade.Direction << std::endl
		<< " OffsetFlag:" << OffsetFlagToString(trade.OffsetFlag) << std::endl
		<< " Price:" << trade.Price << std::endl
		<< " Volume:" << trade.Volume << std::endl
		<< " TradeTime:" << trade.TradeTime << std::endl
		<< " BrokerOrderSeq:" << trade.BrokerOrderSeq << std::endl
		<< " TradeDate:" << trade.TradeDate << std::endl
		<< " TradingDay:" << trade.TradingDay << std::endl;

	return result.str();
}

std::string CommonUtils::ConvertOrderListToString(const std::vector< CThostFtdcOrderField >& list){
	if (list.empty())
		return "";

	std::stringstream result;
	result << std::endl << "------------------------------------------------" << std::endl;

	for (auto iter = list.begin(); iter != list.end(); iter++){
		result << "BrokerID:" << iter->BrokerID << std::endl
			<< " InvestorID:" << iter->InvestorID << std::endl
			<< " UserID:" << iter->UserID << std::endl
			<< " InstrumentID:" << iter->InstrumentID << std::endl
			<< " Direction:" << iter->Direction << std::endl
			<< " CombOffsetFlag:" << iter->CombOffsetFlag << std::endl
			<< " LimitPrice:" << iter->LimitPrice << std::endl
			<< " VolumeTotalOriginal:" << iter->VolumeTotalOriginal << std::endl
			<< " OrderRef:" << iter->OrderRef << std::endl
			<< " ClientID:" << iter->ClientID << std::endl
			<< " OrderStatus:" << iter->OrderStatus << std::endl
			<< " BrokerOrderSeq" << iter->BrokerOrderSeq << std::endl
 			<< " InsertTime:" << iter->InsertTime << std::endl
			<< " OrderSysID:" << iter->OrderSysID << std::endl
			<< " GTDDate:" << iter->GTDDate << std::endl
			<< " TradingDay:" << iter->TradingDay << std::endl
			<< " InsertDate:" << iter->InsertDate << std::endl;
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
		result << "InstrumentID:" << iter->InstrumentID << std::endl
			<< " UserID:" << iter->UserID << std::endl
			<< " TradeID:" << iter->TradeID << std::endl
			<< " Direction:" << iter->Direction << std::endl
			<< " OffsetFlag:" << iter->OffsetFlag << std::endl
			<< " HedgeFlag:" << iter->HedgeFlag << std::endl
			<< " Price:" << iter->Price << std::endl
			<< " Volume:" << iter->Volume << std::endl
			<< " TradeTime:" << iter->TradeTime << std::endl
			<< " TradeType:" << iter->TradeType << std::endl
			<< " OrderSysID:" << iter->OrderSysID << std::endl
			<< " OrderRef:" << iter->OrderRef << std::endl
			<< " OrderLocalID:" << iter->OrderLocalID << std::endl
			<< " BusinessUnit:" << iter->BusinessUnit << std::endl
			<< " SequenceNo:" << iter->SequenceNo << std::endl
			<< " BrokerOrderSeq:" << iter->BrokerOrderSeq << std::endl
			<< " TradeDate:" << iter->TradeDate << std::endl
			<< " TradingDay:" << iter->TradingDay << std::endl;
	}
	result << "-----------------------" << std::endl;
	return result.str();
}


std::string CommonUtils::ConvertPositionFieldToString(const CThostFtdcInvestorPositionField& pos){
	std::stringstream result;

	result << "InstrumentID:" << pos.InstrumentID << std::endl
		<< " TradingDay:" << pos.TradingDay << std::endl
		<< " Position:" << pos.Position << std::endl
		<< " TodayPosition:" << pos.TodayPosition << std::endl
		<< " YdPosition:" << pos.YdPosition << std::endl
		<< " UseMargin:" << pos.UseMargin << std::endl
		<< " Commission:" << pos.Commission << std::endl
		<< " FrozenMargin:" << pos.FrozenMargin << std::endl
		<< " FrozenCommission:" << pos.FrozenCommission << std::endl
		<< " PreMargin:" << pos.PreMargin << std::endl
		<< " PositionProfit:" << pos.PositionProfit << std::endl 
		<< " CloseProfit:" << pos.CloseProfit << std::endl
		<< " OpenVolume:" << pos.OpenVolume << std::endl
		<< " OpenAmount:" << pos.OpenAmount << std::endl
		<< " CloseVolume:" << pos.CloseVolume << std::endl
		<< " CloseAmount:" << pos.CloseAmount << std::endl
		<< " OpenCost:" << pos.OpenCost << std::endl
		<< " PositionCost:" << pos.PositionCost << std::endl << std::endl;

	return result.str();
}

std::string CommonUtils::ConvertInvestorPositionDetailFieldToString(const CThostFtdcInvestorPositionDetailField& posdetail){
	std::stringstream result;

	result << "InstrumentID:" << posdetail.InstrumentID << std::endl
		<< " TradingDay:" << posdetail.TradingDay << std::endl
		<< " Direction:" << posdetail.Direction << std::endl
		<< " ExchangeID:" << posdetail.ExchangeID << std::endl
		<< " Margin:" << posdetail.Margin << std::endl
		<< " OpenDate:" << posdetail.OpenDate << std::endl
		<< " OpenPrice:" << posdetail.OpenPrice << std::endl
		<< " TradeID:" << posdetail.TradeID << std::endl
		<< " Volume:" << posdetail.Volume << std::endl
		<< " CloseAmount:" << posdetail.CloseAmount << std::endl << std::endl;


	return result.str();
}

std::string CommonUtils::ConvertTradingAccountFieldToString(const CThostFtdcTradingAccountField& accField){
	std::stringstream result;

	result << "AccountID:" << accField.AccountID << std::endl
		<< " TradingDay:" << accField.TradingDay << std::endl
		<< " Available:" << accField.Available << std::endl
		<< " Balance:" << accField.Balance << std::endl
		<< " CloseProfit:" << accField.CloseProfit << std::endl
		<< " Commission:" << accField.Commission << std::endl
		<< " CurrMargin:" << accField.CurrMargin << std::endl
		<< " DeliveryMargin:" << accField.DeliveryMargin << std::endl
		<< " Deposit:" << accField.Deposit << std::endl
		<< " ExchangeDeliveryMargin:" << accField.ExchangeDeliveryMargin << std::endl
		<< " ExchangeMargin:" << accField.ExchangeMargin << std::endl
		<< " FrozenCash:" << accField.FrozenCash << std::endl
		<< " FrozenCommission:" << accField.FrozenCommission << std::endl
		<< " FrozenMargin:" << accField.FrozenMargin << std::endl
		<< " PositionProfit:" << accField.PositionProfit << std::endl
		<< " Withdraw:" << accField.Withdraw << std::endl << std::endl;


	return result.str();
}

std::string CommonUtils::ConvertAccountInfoToString(const CThostFtdcTradingAccountField& account){
	std::stringstream result;
        result.precision(10);
	result << "AccountID:" << account.AccountID << std::endl
		<< " FrozenMargin:" << account.FrozenMargin << std::endl
		<< " FrozenCash:" << account.FrozenCash << std::endl
		<< " FrozenCommission:" << account.FrozenCommission << std::endl
		<< " CurrMargin:" << account.CurrMargin << std::endl
		<< " Commission:" << account.Commission << std::endl
		<< " Deposit:" << account.CloseProfit << std::endl
		<< " PositionProfit:" << account.PositionProfit << std::endl
		<< " Balance:" << account.Balance << std::endl
		<< " Available:" << account.Available << std::endl
		<< " Deposit:" << account.Deposit << std::endl
		<< " Withdraw:" << account.Withdraw << std::endl << std::endl;


	return result.str();
}
