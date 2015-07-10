#include "stdafx.h"
#include "KData.h"
#include "DBWrapper.h"

bool KData::IsTableCreated = false;

KData::KData(const TickSet& tickset, size_t duration)
: m_duration(duration)
{
}


KData::~KData()
{
}


int KData::CreateKDataTableIfNotExists(const std::string& dbname, const std::string& tableName){
	if (KData::IsTableCreated == true){
		return 0;
	}
	else
	{
		KData::IsTableCreated = true;
		const char* sqltempl = "CREATE TABLE IF NOT EXISTS `%s`.`%s` (\
			`id` INT NOT NULL AUTO_INCREMENT, \
			`Date` DATE NULL, \
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
			`UpperLimitPrice` DOUBLE NULL, \
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
			`uuid` BIGINT NULL, \
			PRIMARY KEY(`id`));";
		char sqlbuf[2046];
		sprintf_s(sqlbuf, sqltempl, dbname.c_str(), tableName.c_str());
		DBWrapper db;
		return db.ExecuteNoResult(sqlbuf);
	}
}