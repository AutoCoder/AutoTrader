#include "stdafx.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "ThostFtdcUserApiStruct.h"
#include "DBWrapper.h"
#include <sstream>
#include <iostream>
#include <time.h>

namespace {
	int createTickTable(const std::string& dbname, const std::string& tableName){
		const char* sqltempl = "CREATE TABLE `%s`.`%s` ( \
			`id` INT NOT NULL AUTO_INCREMENT, \
			`Date` DATETIME NULL, \
			`InstrumentID` VARCHAR(32) NULL, \
			`ExchangeID` VARCHAR(16) NULL, \
			`ExchangeInstID` VARCHAR(32) NULL, \
			`LastPrice` DOUBLE NULL, \
			`PreSettlementPrice` DOUBLE NULL, \
			`PreClosePrice` DOUBLE NULL, \
			`PreOpenInterest` DOUBLE NULL, \
			`OpenPrice` DOUBLE NULL, \
			`HighestPrice` DOUBLE NULL, \
			`LowestPrice` DOUBLE NULL, \
			`Volume` DOUBLE NULL, \
			`Turnover` DOUBLE NULL, \
			`OpenInterest` DOUBLE NULL, \
			`ClosePrice` DOUBLE NULL, \
			`SettlementPrice` DOUBLE NULL, \
			`LowerLimitPrice` DOUBLE NULL, \
			`PreDelta` DOUBLE NULL, \
			`CurrDelta` DOUBLE NULL, \
			`UpdateTime` TIME NULL, \
			`UpdateMillisec` INT NULL, \
			`BidPrice1` DOUBLE NULL, \
			`BidVolume1` DOUBLE NULL, \
			`AskPrice1` DOUBLE NULL, \
			`AskVolume1` DOUBLE NULL, \
			`BidPrice2` DOUBLE NULL, \
			`BidVolume2` DOUBLE NULL, \
			`AskPrice2` DOUBLE NULL, \
			`AskVolume2` DOUBLE NULL, \
			`BidPrice3` DOUBLE NULL, \
			`BidVolume3` DOUBLE NULL, \
			`AskPrice3` DOUBLE NULL, \
			`AskVolume3` DOUBLE NULL, \
			`BidPrice4` DOUBLE NULL, \
			`BidVolume4` DOUBLE NULL, \
			`AskPrice4` DOUBLE NULL, \
			`AskVolume4` DOUBLE NULL, \
			`BidPrice5` DOUBLE NULL, \
			`BidVolume5` DOUBLE NULL, \
			`AskPrice5` DOUBLE NULL, \
			`AskVolume5` DOUBLE NULL, \
			`AveragePrice` DOUBLE NULL, \
			`ActionDay` DATE NULL, \
			`k3m` DOUBLE NULL, \
			`k5m` DOUBLE NULL, \
			PRIMARY KEY(`id`));";
		char sqlbuf[2046];
		sprintf_s(sqlbuf, sqltempl, dbname, tableName);
		return DBWrapper::GetDBWrapper().ExecuteNoResult(sqlbuf);
	}
}


CThostFtdcDepthMDFieldWrapper::CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p):
	m_innerPtr(new CThostFtdcDepthMarketDataField())
	m_k5m(0.0),
	m_k3m(0.0)
{
	memcpy(m_innerPtr, p, sizeof(CThostFtdcDepthMarketDataField));
}


CThostFtdcDepthMDFieldWrapper::~CThostFtdcDepthMDFieldWrapper()
{
	delete m_innerPtr;
}

double CThostFtdcDepthMDFieldWrapper::turnOver() const{
	return m_innerPtr->Turnover;
}

int CThostFtdcDepthMDFieldWrapper::volume() const{
	return m_innerPtr->Volume;
}

int CThostFtdcDepthMDFieldWrapper::toTimeStamp() const{
	const char* pDate = m_innerPtr->TradingDay;

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

	const char* pTime = m_innerPtr->UpdateTime;

	szHour[0] = *pDate++;
	szHour[1] = *pDate++;
	szHour[2] = 0x0;
	pDate++;

	szMin[0] = *pDate++;
	szMin[1] = *pDate++;
	szMin[2] = 0x0;
	pDate++;

	szSec[0] = *pDate++;
	szSec[1] = *pDate++;
	szSec[2] = 0x0;

	tm tmObj;

	tmObj.tm_year = atoi(szYear) - 1970;
	tmObj.tm_mon = atoi(szMonth) - 1;
	tmObj.tm_mday = atoi(szDay);
	tmObj.tm_hour = atoi(szHour);
	tmObj.tm_min = atoi(szMin);
	tmObj.tm_sec = atoi(szSec);
	tmObj.tm_isdst = -1;

	int&& ret = mktime(&tmObj) * 2 + m_innerPtr->UpdateMillisec / 500;
	return ret;
}

void CThostFtdcDepthMDFieldWrapper::serializeToDB() const {
	std::stringstream sql;
	sql << "INSERT INTO `test` (` ";
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
	sql << "ActionDay";
	sql << ") VALUES(";
	sql << m_innerPtr->TradingDay << ", ";
	sql << m_innerPtr->InstrumentID << ", ";
	sql << m_innerPtr->ExchangeID << ", ";
	sql << m_innerPtr->ExchangeInstID << ", ";
	sql << m_innerPtr->LastPrice << ", ";
	sql << m_innerPtr->PreSettlementPrice << ", ";
	sql << m_innerPtr->PreClosePrice << ", ";
	sql << m_innerPtr->PreOpenInterest << ", ";
	sql << m_innerPtr->OpenPrice << ", ";
	sql << m_innerPtr->HighestPrice << ", ";
	sql << m_innerPtr->LowestPrice << ", ";
	sql << m_innerPtr->Volume << ", ";
	sql << m_innerPtr->Turnover << ", ";
	sql << m_innerPtr->OpenInterest << ", ";
	sql << m_innerPtr->ClosePrice << ", ";
	sql << m_innerPtr->SettlementPrice << ", ";
	sql << m_innerPtr->UpperLimitPrice << ", ";
	sql << m_innerPtr->LowerLimitPrice << ", ";
	sql << m_innerPtr->PreDelta << ", ";
	sql << m_innerPtr->CurrDelta << ", ";
	sql << m_innerPtr->UpdateTime << ", ";
	sql << m_innerPtr->UpdateMillisec << ", ";
	sql << m_innerPtr->BidPrice1 << ", ";
	sql << m_innerPtr->BidVolume1 << ", ";
	sql << m_innerPtr->AskPrice1 << ", ";
	sql << m_innerPtr->AskVolume1 << ", ";
	sql << m_innerPtr->BidPrice2 << ", ";
	sql << m_innerPtr->BidVolume2 << ", ";
	sql << m_innerPtr->AskPrice2 << ", ";
	sql << m_innerPtr->AskVolume2 << ", ";
	sql << m_innerPtr->BidPrice3 << ", ";
	sql << m_innerPtr->BidVolume3 << ", ";
	sql << m_innerPtr->AskPrice3 << ", ";
	sql << m_innerPtr->AskVolume3 << ", ";
	sql << m_innerPtr->BidPrice4 << ", ";
	sql << m_innerPtr->BidVolume4 << ", ";
	sql << m_innerPtr->AskPrice4 << ", ";
	sql << m_innerPtr->AskVolume4 << ", ";
	sql << m_innerPtr->BidPrice5 << ", ";
	sql << m_innerPtr->BidVolume5 << ", ";
	sql << m_innerPtr->AskPrice5 << ", ";
	sql << m_innerPtr->AskVolume5 << ", ";
	sql << m_innerPtr->AveragePrice << ", ";
	sql << m_innerPtr->ActionDay << ")";
	//"INSERT INTO `test` (`name`) VALUES (1234) 
	std::cerr << sql.str() << std::endl;
	DBWrapper::GetDBWrapper().ExecuteNoResult(sql.str());
}
