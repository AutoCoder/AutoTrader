#include "stdafx.h"
#include "DBWrapper.h"
#include <winsock2.h>
#include "mysqlwrapper.h"
#include <iostream>
#include "config.h"

const char * DBWrapper::DBHost = "127.0.0.1";
const int DBWrapper::DBPort = 3306;
const char * DBWrapper::DBName = "qihuo";
const char * DBWrapper::User = "root";
const char * DBWrapper::Password = "wodemima";


namespace DBUtils{

int CreateTickTableIfNotExists(const std::string& dbname, const std::string& tableName){
	const char* sqltempl = "CREATE TABLE IF NOT EXISTS `%s`.`%s` ( \
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
	sprintf_s(sqlbuf, sqltempl, dbname.c_str(), tableName.c_str());
	return DBWrapper::GetDBWrapper().ExecuteNoResult(sqlbuf);
}

};


DBWrapper& DBWrapper::GetDBWrapper(){
	static DBWrapper db;

	//for now, it doesn't support multi-thread
	return db;
}

DBWrapper::DBWrapper()
	:m_MysqlImpl(new mysql_db())
{
	if (-1 == m_MysqlImpl->mysql_open(Config::Instance()->DBHost().c_str() \
		, Config::Instance()->DBUser().c_str() \
		, Config::Instance()->DBPassword().c_str() \
		, Config::Instance()->DBName().c_str() \
		, Config::Instance()->DBPort()))
	{
		std::cerr << m_MysqlImpl->mysql_lasterror() << std::endl;
	}
		
	//ExecuteNoResult("ExecuteNoResult")
}

DBWrapper::~DBWrapper()
{
}


int DBWrapper::ExecuteNoResult(const std::string& sql){
	int ret = m_MysqlImpl->mysql_noResult_query(sql.c_str());
	if (-1 == ret)		std::cerr << m_MysqlImpl->mysql_lasterror() << std::endl;
	return ret;
}

