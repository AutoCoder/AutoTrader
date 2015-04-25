#pragma once
#include <memory>
#include <string>
class mysql_db;

class DBWrapper
{
public:
	static DBWrapper& GetDBWrapper();
	~DBWrapper();
	void ExecuteNoResult(const std::string& sql);
private:
	DBWrapper();
	std::shared_ptr<mysql_db> m_MysqlImpl;
	static const char *DBHost;
	static const int DBPort;
	static const char *DBName;
	static const char *User;
	static const char *Password;
};

