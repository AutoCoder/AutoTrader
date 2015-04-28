#include "stdafx.h"
#include "DBWrapper.h"
#include <winsock2.h>
#include "mysqlwrapper.h"
#include <iostream>

const char * DBWrapper::DBHost = "121.40.99.4";
const int DBWrapper::DBPort = 3306;
const char * DBWrapper::DBName = "testdb";
const char * DBWrapper::User = "root";
const char * DBWrapper::Password = "wodemima";

DBWrapper& DBWrapper::GetDBWrapper(){
	static DBWrapper db;

	//for now, it doesn't support multi-thread
	return db;
}

DBWrapper::DBWrapper()
	:m_MysqlImpl(new mysql_db())
{
	if (-1 == m_MysqlImpl->mysql_open(DBHost, User, Password, DBName, DBPort))
		std::cerr << m_MysqlImpl->mysql_lasterror() << std::endl;
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