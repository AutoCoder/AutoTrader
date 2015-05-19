#include "stdafx.h"
#include "TechVec.h"
#include "config.h"
#include "DBWrapper.h"
#include <sstream>


TechVec::TechVec(long long uuid, const std::string& instrumentID)
	: m_id(uuid)
	, m_instrumentId(instrumentID)
{
	for (int i = 0; i < sizeof(m_ticktype) / sizeof(TickType); i++){
		m_ticktype[i] = TickType::Commom;
	}
}


TechVec::~TechVec()
{
}


void TechVec::serializeToDB(DBWrapper& db){

	std::string&& tableName = m_instrumentId + "TechVec";

	DBUtils::CreateTechVecTableIfNotExists(Config::Instance()->DBName(), tableName);

	std::stringstream sql;
	sql << "INSERT INTO `" << tableName << "` (`";
	sql << "uuid" << "`,`";
	sql << "k5m" << "`,`";
	sql << "k3m" << "`,`";
	sql << "Strategy1" << "`,`";
	sql << "Strategy2" << "`,`";
	sql << "Strategy3" << "`,`";
	sql << "Strategy4" << "`,`";
	sql << "Strategy5" << "`,`";
	sql << "Strategy6" << "`,`";
	sql << "Strategy7" << "`,`";
	sql << "Strategy8" << "`";
	sql << ") VALUES(";
	sql << m_id << ", ";
	sql << m_k5m << ", ";
	sql << m_k3m << ", ";
	sql << (int)m_ticktype[0] << ", ";
	sql << (int)m_ticktype[1] << ", ";
	sql << (int)m_ticktype[2] << ", ";
	sql << (int)m_ticktype[3] << ", ";
	sql << (int)m_ticktype[4] << ", ";
	sql << (int)m_ticktype[5] << ", ";
	sql << (int)m_ticktype[6] << ", ";
	sql << (int)m_ticktype[7] << ")";

	//std::cerr << sql.str() << std::endl;
	db.ExecuteNoResult(sql.str());
}

k3UpThroughK5TechVec::k3UpThroughK5TechVec(long long uuid, const std::string& instrumentID)
	: StrategyTechVec(uuid, instrumentID)
	, m_ticktype(TickType::Commom)
{
}

void k3UpThroughK5TechVec::serializeToDB(DBWrapper& db)
{
	std::string&& tableName = m_instrumentId + "k3UpThroughK5";

	DBUtils::CreateK3K5StrategyTableIfNotExists(Config::Instance()->DBName(), tableName);

	std::stringstream sql;
	sql << "INSERT INTO `" << tableName << "` (`";
	sql << "uuid" << "`,`";
	sql << "k5m" << "`,`";
	sql << "k3m" << "`,`";
	sql << "Ticktype" << "`";
	sql << ") VALUES(";
	sql << m_id << ", ";
	sql << m_k5m << ", ";
	sql << m_k3m << ", ";
	sql << (int)m_ticktype << ")";

	//std::cerr << sql.str() << std::endl;
	db.ExecuteNoResult(sql.str());
}