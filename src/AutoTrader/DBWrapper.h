#ifndef DB_WRAPPER_H
#define DB_WRAPPER_H

#include <memory>
#include <map>
#include <vector>
class mysql_db;

class DBWrapper
{
public:
	DBWrapper();
	~DBWrapper();
	int ExecuteNoResult(const std::string& sql);
	int Query(const std::string& sql, std::map<int, std::vector<std::string> > & map_results);
private:
	std::shared_ptr<mysql_db> m_MysqlImpl;
};



class DBUtils
{
public:
	static int CreateTickTableIfNotExists(const std::string& dbname, const std::string& tableName);

private:
	static std::map<std::string, bool> m_dict;
};

#endif