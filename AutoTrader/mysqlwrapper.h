#ifndef MYSQL_H_
#define MYSQL_H_

#include "mysql.h"
#include <string>

#include <map>
#include <vector>

class mysql_db
{
public:
	mysql_db();
	~mysql_db();
public:

	int mysql_open(const char * host, const char * user, const char * password, const char * database, unsigned int port);

	int mysql_noResult_query(const char * sql);

	int mysql_select_query(const char * sql, std::map<int, std::vector<std::string>> & map_results);

	int mysql_select_SingleLine_query(const char * sql, std::vector<std::string> & v_results);

	std::string mysql_lasterror();
private:
	MYSQL sqlCon;
	MYSQL_RES *m_pResult;
	MYSQL_ROW  m_Row;
};

#endif