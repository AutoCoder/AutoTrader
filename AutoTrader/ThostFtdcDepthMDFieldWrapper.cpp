#include "stdafx.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "DBWrapper.h"
#include <sstream>
#include <iostream>
#include <time.h>

bool CThostFtdcDepthMDFieldWrapper::firstlanuch = true;

CThostFtdcDepthMDFieldWrapper::CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p):
	m_k5m(0.0),
	m_k3m(0.0)
{
	memcpy(&m_MdData, p, sizeof(CThostFtdcDepthMarketDataField));
}

CThostFtdcDepthMDFieldWrapper::~CThostFtdcDepthMDFieldWrapper()
{
}

long long CThostFtdcDepthMDFieldWrapper::toTimeStamp() const{
	const char* pDate = m_MdData.TradingDay;

	char szYear[5], szMonth[3], szDay[3], szHour[3], szMin[3], szSec[3];

	szYear[0] = *pDate++;
	szYear[1] = *pDate++;
	szYear[2] = *pDate++;
	szYear[3] = *pDate++;
	szYear[4] = 0x0;

	szMonth[0] = *pDate++;
	szMonth[1] = *pDate++;
	szMonth[2] = 0x0;

	szDay[0] = *pDate++;
	szDay[1] = *pDate++;
	szDay[2] = 0x0;

	const char* pTime =  m_MdData.UpdateTime;

	szHour[0] = *pTime++;
	szHour[1] = *pTime++;
	szHour[2] = 0x0;
	pTime++;

	szMin[0] = *pTime++;
	szMin[1] = *pTime++;
	szMin[2] = 0x0;
	pTime++;

	szSec[0] = *pTime++;
	szSec[1] = *pTime++;
	szSec[2] = 0x0;

	tm tmObj;

	tmObj.tm_year = atoi(szYear) - 1900;
	tmObj.tm_mon = atoi(szMonth) - 1;
	tmObj.tm_mday = atoi(szDay);
	tmObj.tm_hour = atoi(szHour);
	tmObj.tm_min = atoi(szMin);
	tmObj.tm_sec = atoi(szSec);
	tmObj.tm_isdst = -1;

	long long ret = mktime(&tmObj) * 2 +  m_MdData.UpdateMillisec / 500;
	return ret;
}

void CThostFtdcDepthMDFieldWrapper::serializeToDB() const {
	std::string tableName(m_MdData.InstrumentID);

	if (firstlanuch){
		int&& ret = DBUtils::CreateTickTableIfNotExists(DBWrapper::DBName, tableName);
		if (ret == 0)
			firstlanuch = false;
	}

	std::stringstream sql;
	sql << "INSERT INTO `" << tableName << "` (` ";
	sql << "Date" << "`,`";
	sql << "InstrumentID" << "`,`";
	sql << "ExchangeID" << "`,`";
	sql << "ExchangeInstID" << "`,`";
	sql << "LastPrice" << "`,`";
	sql << "PreSettlementPrice" << "`,`";
	sql << "PreClosePrice" << "`,`";
	sql << "PreOpenInterest" << "`,`";
	sql << "OpenPrice" << "`,`";
	sql << "HighestPrice" << "`,`";
	sql << "LowestPrice" << "`,`";
	sql << "Volume" << "`,`";
	sql << "Turnover" << "`,`";
	sql << "OpenPrice" << "`,`";
	sql << "OpenInterest" << "`,`";
	sql << "ClosePrice" << "`,`";
	sql << "SettlementPrice" << "`,`";
	sql << "UpperLimitPrice" << "`,`";
	sql << "LowerLimitPrice" << "`,`";
	sql << "PreDelta" << "`,`";
	sql << "CurrDelta"<< "`,`";
	sql << "UpdateTime"<< "`,`";
	sql << "UpdateMillisec" << "`,`";
	sql << "BidPrice1" << "`,`";
	sql << "BidVolume1" << "`,`";
	sql << "AskPrice1" << "`,`";
	sql << "AskVolume1" << "`,`";
	sql << "BidPrice2" << "`,`";
	sql << "BidVolume2" << "`,`";
	sql << "AskPrice2" << "`,`";
	sql << "AskVolume2" << "`,`";
	sql << "BidPrice3" << "`,`";
	sql << "BidVolume3" << "`,`";
	sql << "AskPrice3" << "`,`";
	sql << "AskVolume3" << "`,`";
	sql << "BidPrice4" << "`,`";
	sql << "BidVolume4" << "`,`";
	sql << "AskPrice4" << "`,`";
	sql << "AskVolume4" << "`,`";
	sql << "BidPrice5" << "`,`";
	sql << "BidVolume5" << "`,`";
	sql << "AskPrice5" << "`,`";
	sql << "AskVolume5" << "`,`";
	sql << "AveragePrice" << "`,`";
	sql << "ActionDay" << "`,`";
	sql << "k3m" << "`,`";
	sql << "k5m" << "`";
	sql << ") VALUES(";
	sql <<  m_MdData.TradingDay << ", ";
	sql <<  m_MdData.InstrumentID << ", ";
	sql <<  m_MdData.ExchangeID << ", ";
	sql <<  m_MdData.ExchangeInstID << ", ";
	sql <<  m_MdData.LastPrice << ", ";
	sql <<  m_MdData.PreSettlementPrice << ", ";
	sql <<  m_MdData.PreClosePrice << ", ";
	sql <<  m_MdData.PreOpenInterest << ", ";
	sql <<  m_MdData.OpenPrice << ", ";
	sql <<  m_MdData.HighestPrice << ", ";
	sql <<  m_MdData.LowestPrice << ", ";
	sql <<  m_MdData.Volume << ", ";
	sql <<  m_MdData.Turnover << ", ";
	sql <<  m_MdData.OpenInterest << ", ";
	sql <<  m_MdData.ClosePrice << ", ";
	sql <<  m_MdData.SettlementPrice << ", ";
	sql <<  m_MdData.UpperLimitPrice << ", ";
	sql <<  m_MdData.LowerLimitPrice << ", ";
	sql <<  m_MdData.PreDelta << ", ";
	sql <<  m_MdData.CurrDelta << ", ";
	sql <<  m_MdData.UpdateTime << ", ";
	sql <<  m_MdData.UpdateMillisec << ", ";
	sql <<  m_MdData.BidPrice1 << ", ";
	sql <<  m_MdData.BidVolume1 << ", ";
	sql <<  m_MdData.AskPrice1 << ", ";
	sql <<  m_MdData.AskVolume1 << ", ";
	sql <<  m_MdData.BidPrice2 << ", ";
	sql <<  m_MdData.BidVolume2 << ", ";
	sql <<  m_MdData.AskPrice2 << ", ";
	sql <<  m_MdData.AskVolume2 << ", ";
	sql <<  m_MdData.BidPrice3 << ", ";
	sql <<  m_MdData.BidVolume3 << ", ";
	sql <<  m_MdData.AskPrice3 << ", ";
	sql <<  m_MdData.AskVolume3 << ", ";
	sql <<  m_MdData.BidPrice4 << ", ";
	sql <<  m_MdData.BidVolume4 << ", ";
	sql <<  m_MdData.AskPrice4 << ", ";
	sql <<  m_MdData.AskVolume4 << ", ";
	sql <<  m_MdData.BidPrice5 << ", ";
	sql <<  m_MdData.BidVolume5 << ", ";
	sql <<  m_MdData.AskPrice5 << ", ";
	sql <<  m_MdData.AskVolume5 << ", ";
	sql <<  m_MdData.AveragePrice << ", ";
	sql <<  m_MdData.ActionDay << ", ";
	sql <<  m_k3m << ", ";
	sql <<  m_k5m << ")";
	//"INSERT INTO `test` (`name`) VALUES (1234) 
	std::cerr << sql.str() << std::endl;
	DBWrapper::GetDBWrapper().ExecuteNoResult(sql.str());
}
