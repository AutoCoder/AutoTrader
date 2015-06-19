#ifndef MD_CONFIG_H_
#define MD_CONFIG_H_
#include <string>
#include <map>
#include <vector>

 //char mdFront[] = "tcp://180.168.146.181:10210";
 //char mdFront[]   ="tcp://asp-sim1-front1.financial-trading-platform.com:41213";
 //char tradeFront[]="tcp://asp-sim1-front1.financial-trading-platform.com:41205";
 //char riskFront[] ="tcp://asp-sim1-front1.financial-trading-platform.com:50001";

//char mdFront[]   ="tcp://asp-sim2-md1.financial-trading-platform.com:26213";
//char tradeFront[]="tcp://asp-sim2-front1.financial-trading-platform.com:26205";

//char	appId[] = "2030";			
//char userId[] = "0000000624";		
//char riskUser[]="demo";		
//char  passwd[] = "asdfgh";	

//charappId[] = "2030";			
//char userId[] = "dongheng1986";		
//char riskUser[]="demo";		
//char  passwd[] = "dh851223";	

 //char	appId[] = "0292";			
 //char userId[] = "00127";				
 //char  passwd[] = "asdfgh";	
#ifdef CONF_INI
 class ini_parser;
#endif
class StrategyMetaData;

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
	std::vector<StrategyMetaData> StrategySet(void);
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