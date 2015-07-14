#include "stdafx.h"
#include "KData.h"
#include "DBWrapper.h"
#include "config.h"
#include <limits>  
#include <sstream>

bool KData::IsTableCreated = false;

KData::KData(const TickSet& tickset, size_t duration)
: m_duration(duration)
, m_LowestPrice(std::numeric_limits<double>::max())
, m_HighestPrice(0)
, m_volume(0)
, m_Turnover(0)
{
	for (auto item : tickset){
		m_volume += item.Volume();
		m_Turnover += item.TurnOver();
		m_LowestPrice = m_LowestPrice > item.LowestPrice() ? item.LowestPrice() : m_LowestPrice;
		m_HighestPrice = m_HighestPrice < item.HighestPrice() ? item.HighestPrice() : m_HighestPrice;
	}
	m_averagePrice = m_Turnover / m_volume;
	m_timestamp = tickset.front().FirstSecondsTimeStamp();
	m_OpenPrice = tickset.front().OpenPrice();
	m_PreSettlementPrice = tickset.front().PreSettlementPrice();

	m_PreClosePrice = tickset.front().PreClosePrice();
	m_UpperLimitPrice = tickset.front().UpperLimitPrice();
	m_LowerLimitPrice = tickset.front().LowerLimitPrice();
	m_BidPrice1 = tickset.back().BidPrice1();
	m_BidVolume1 = tickset.back().BidVolume1();
	m_AskPrice1 = tickset.back().AskPrice1();
	m_AskVolume1 = tickset.back().AskVolume1();
	m_ClosePrice = tickset.back().LastPrice();
	m_LastPrice = tickset.back().LastPrice();
	m_SettlementPrice = tickset.back().SettlementPrice();

	m_InstrumentID = tickset.front().InstrumentId();
	m_time = tickset.front().Time();
	size_t len = m_time.length();
	m_time[len - 1] = '0';
	m_time[len - 2] = '0';
}


KData::~KData()
{
}


void KData::serializeToDB(DBWrapper& db){

	std::string tableName(m_InstrumentID);
	tableName += "_1m";

	KData::CreateKDataTableIfNotExists(Config::Instance()->DBName(), tableName);

	std::stringstream sql;
	sql << "INSERT INTO `" << tableName << "` (`";
	sql << "Time" << "`,`";
	sql << "InstrumentID" << "`,`";
	sql << "LastPrice" << "`,`";
	sql << "PreSettlementPrice" << "`,`";
	sql << "PreClosePrice" << "`,`";
	sql << "OpenPrice" << "`,`";
	sql << "HighestPrice" << "`,`";
	sql << "LowestPrice" << "`,`";
	sql << "Volume" << "`,`";
	sql << "Turnover" << "`,`";
	sql << "ClosePrice" << "`,`";
	sql << "SettlementPrice" << "`,`";
	sql << "UpperLimitPrice" << "`,`";
	sql << "LowerLimitPrice" << "`,`";
	sql << "BidPrice1" << "`,`";
	sql << "BidVolume1" << "`,`";
	sql << "AskPrice1" << "`,`";
	sql << "AskVolume1" << "`,`";
	sql << "AveragePrice" << "`,`";
	sql << "uuid" << "`";
	sql << ") VALUES(\"";
	sql << m_time << "\", \"";
	sql << m_InstrumentID << "\", ";
	sql << m_LastPrice << ", ";
	sql << m_PreSettlementPrice << ", ";
	sql << m_PreClosePrice << ", ";
	sql << m_OpenPrice << ", ";
	sql << m_HighestPrice << ", ";
	sql << m_LowestPrice << ", ";
	sql << m_volume << ", ";
	sql << m_Turnover << ", ";
	sql << m_ClosePrice << ", ";
	sql << m_SettlementPrice << ", ";
	sql << m_UpperLimitPrice << ", ";
	sql << m_LowerLimitPrice << ", ";
	sql << m_BidPrice1 << ", ";
	sql << m_BidVolume1 << ", ";
	sql << m_AskPrice1 << ", ";
	sql << m_AskVolume1 << ", ";
	sql << m_averagePrice << ", ";
	sql << m_timestamp << ")";
	//"INSERT INTO `test` (`name`) VALUES (1234) 
	//std::cerr << sql.str() << std::endl;
	db.ExecuteNoResult(sql.str());
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
			`Time` VARCHAR(32) NULL, \
			`uuid` BIGINT NULL, \
			`InstrumentID` VARCHAR(32) NULL, \
			`LastPrice` DOUBLE NULL, \
			`PreSettlementPrice` DOUBLE NULL, \
			`PreClosePrice` DOUBLE NULL, \
			`OpenPrice` DOUBLE NULL, \
			`HighestPrice` DOUBLE NULL, \
			`LowestPrice` DOUBLE NULL, \
			`Volume` DOUBLE NULL, \
			`Turnover` DOUBLE NULL, \
			`ClosePrice` DOUBLE NULL, \
			`SettlementPrice` DOUBLE NULL, \
			`UpperLimitPrice` DOUBLE NULL, \
			`LowerLimitPrice` DOUBLE NULL, \
			`BidPrice1` DOUBLE NULL, \
			`BidVolume1` DOUBLE NULL, \
			`AskPrice1` DOUBLE NULL, \
			`AskVolume1` DOUBLE NULL, \
			`AveragePrice` DOUBLE NULL, \
			PRIMARY KEY(`id`));";
		char sqlbuf[2046];
		sprintf_s(sqlbuf, sqltempl, dbname.c_str(), tableName.c_str());
		DBWrapper db;
		return db.ExecuteNoResult(sqlbuf);
	}
}