#ifndef CTP_CONFIG_H_
#define CTP_CONFIG_H_
#include <string>
#include <map>
#include <vector>


#ifdef CONF_INI
 class ini_parser;
#endif
struct StrategyMetaData;

class Config {
public:
	~Config();
	static Config* Instance();
	std::string CtpBrokerID();
	std::string CtpUserID();
	std::string CtpPassword();
	std::string CtpMdFront();
	std::string CtpTradeFront();
	std::string CtpInstrumentIDs();
	std::string CtpStrategy(const std::string& instrumentID);

	std::string DBHost();
	int DBPort();
	std::string DBName();
	std::string DBUser();
	std::string DBPassword();

	std::string ProductName();
	std::vector<StrategyMetaData> StrategySet();
	bool RecordModeOn();
private:
	Config(const std::string& pathName);

private:
	static Config* m_instance;
#ifdef CONF_INI
	ini_parser* parser_instance;
#endif
	std::string m_filepath;
	std::string m_ctp_brokerId;
	std::string m_ctp_userId;
	std::string m_ctp_password;
	std::string m_ctp_mdfront;
	std::string m_ctp_tradefront;
	std::string m_ctp_riskfront;
	std::string m_instruments;
	std::vector<StrategyMetaData> m_strategies;
	std::map<std::string/*instrument*/, std::string/*stratgy name*/> m_strategymap;
	std::string m_db_host;
	int m_db_port;
	std::string m_db_name;
	std::string m_db_user;
	std::string m_db_password;
	std::string m_db_productName;
	bool m_recodeMode;

};


#endif