#ifndef DB_WRAPPER_H
#define DB_WRAPPER_H
#include <memory>
#include <string>
class mysql_db;

class DBWrapper
{
public:
	static DBWrapper& GetDBWrapper();
	~DBWrapper();
	int ExecuteNoResult(const std::string& sql);

public:
	static const char *DBName;

private:
	DBWrapper();
	std::shared_ptr<mysql_db> m_MysqlImpl;
	static const char *DBHost;
	static const int DBPort;
	static const char *User;
	static const char *Password;
};

namespace DBUtils{

	int CreateTickTableIfNotExists(const std::string& dbname, const std::string& tableName);

}

#endif