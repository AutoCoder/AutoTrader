#ifndef CTP_CONFIGV2_H_
#define CTP_CONFIGV2_H_

#include <map>
#include <vector>

class ConfigV2 {
public:
	~ConfigV2();
	static ConfigV2* Instance();
	std::string CtpBrokerID() const ;
	std::string CtpUserID() const;
	std::string CtpPassword() const;
	std::string CtpMdFront() const;
	std::string CtpTradeFront() const;

	std::string DBHost() const;
	int DBPort();
	std::string DBName() const;
	std::string DBUser() const;
	std::string DBPassword() const;
	std::string ProductName() const;

private:
	ConfigV2(const ConfigV2&) = delete;
	ConfigV2& operator = (const ConfigV2&) = delete;
	ConfigV2(const std::string& pathName);

private:
	static ConfigV2* m_instance;
	std::string m_filepath;
	std::string m_ctp_mdfront;
	std::string m_ctp_tradefront;
	std::string m_ctp_riskfront;

	std::vector<std::string> m_strategies;
	std::string m_db_host;
	int m_db_port;
	std::string m_db_name;
	std::string m_db_user;
	std::string m_db_password;
	std::string m_db_productName;
};

#endif