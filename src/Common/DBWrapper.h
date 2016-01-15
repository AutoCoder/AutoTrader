#ifndef DB_WRAPPER_H
#define DB_WRAPPER_H

#include <memory>
#include <map>
#include <vector>
#include "Common.h"

class mysql_db;

class COMMON_API DBWrapper
{
public:
	DBWrapper();
	~DBWrapper();
	int ExecuteNoResult(const std::string& sql);
	int Query(const std::string& sql, std::map<int, std::vector<std::string> > & map_results);
private:
	std::shared_ptr<mysql_db> m_MysqlImpl;
};



class COMMON_API DBUtils
{
public:
	static int CreateTickTableIfNotExists(const std::string& dbname, const std::string& tableName);

private:
	static std::map<std::string, bool> m_dict;
};

#endif