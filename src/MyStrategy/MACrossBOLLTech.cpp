#include <sstream>
#include "MACrossBOLLTech.h"
#include "DBWrapper.h"
#include "Config.h"
#include "crossplatform.h"


MACrossBOLLTech::MACrossBOLLTech(CrossStratgyType type, size_t shortMA, size_t longMA, long long uuid, const std::string& instrumentID, const std::string& time, double lastprice)
: m_type(type)
, m_id(uuid)
, m_ticktype(TickType::Commom)
, m_lastprice(lastprice)
, m_shortMA(shortMA)
, m_longMA(longMA)
, m_maTech()
, m_bollTech()
{
	STRCPY(m_time, time.c_str());
	STRCPY(m_instrumentId, instrumentID.c_str());
}


MACrossBOLLTech::~MACrossBOLLTech()
{
}


bool MACrossBOLLTech::IsTriggerPoint() const
{
	return /*m_bollTech.IsTriggerPoint() &&*/ m_maTech.IsTriggerPoint();
}

bool MACrossBOLLTech::IsTableCreated = false;

void MACrossBOLLTech::serializeToDB(DBWrapper& db, const std::string& mark)
{
	std::stringstream tableName;
	tableName << std::string(m_instrumentId);
	tableName << "_";
	tableName << StratgyType::toString(this->m_type);
	tableName << "_MA" << m_shortMA << "_Cross_MA" << m_longMA << "_BOLL_";
	tableName << mark;


	MACrossBOLLTech::CreateTableIfNotExists(Config::Instance()->DBName(), tableName.str());

	std::stringstream sql;
	sql.precision(12);
	sql << "INSERT INTO `" << tableName.str() << "` (`";
	sql << "uuid" << "`,`";
	sql << "LongMA" << "`,`";
	sql << "ShortMA" << "`,`";
	sql << "BollUp" << "`,`";
	sql << "BollMid" << "`,`";
	sql << "BollDown" << "`,`";
	sql << "Variance" << "`,`";
	sql << "Ticktype" << "`,`";
	sql << "Time" << "`,`";
	sql << "LastPrice" << "`";
	sql << ") VALUES(";
	sql << m_id << ", ";
	sql << m_maTech.mLongMAVal << ", ";
	sql << m_maTech.mShortMAVal << ", ";
	sql << m_bollTech.BollUp() << ", ";
	sql << m_bollTech.BollMid() << ", ";
	sql << m_bollTech.BollDown() << ", ";
	sql << m_bollTech.Variance() << ", ";
	sql << (int)m_ticktype << ", \"";
	sql << m_time << "\", ";
	sql << m_lastprice << ")";

	//std::cerr << sql.str() << std::endl;
	db.ExecuteNoResult(sql.str());
}

int MACrossBOLLTech::CreateTableIfNotExists(const std::string& dbname, const std::string& tableName){
	if (MACrossBOLLTech::IsTableCreated == true){
		return 0;
	}
	else
	{
		MACrossBOLLTech::IsTableCreated = true;
		const char* sqltempl = "CREATE TABLE IF NOT EXISTS `%s`.`%s` (\
								`id` INT NOT NULL AUTO_INCREMENT, \
								`uuid` BIGINT NOT NULL, \
								`LongMA` Double(20,5) NULL, \
								`ShortMA` Double(20,5) NULL, \
								`BollUp` Double(20,5) NULL, \
								`BollMid` Double(20,5) NULL, \
								`BollDown` Double(20,5) NULL, \
								`Variance` Double(20,5) NULL, \
								`Ticktype` int NULL, \
								`Time` VARCHAR(64) NULL, \
								`LastPrice` Double NULL, \
								PRIMARY KEY(`id`));";
		char sqlbuf[2046];
		SPRINTF(sqlbuf, sqltempl, dbname.c_str(), tableName.c_str());
		DBWrapper db;
		return db.ExecuteNoResult(sqlbuf);
	}
}