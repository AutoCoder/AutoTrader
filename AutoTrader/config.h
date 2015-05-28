#ifndef MD_CONFIG_H_
#define MD_CONFIG_H_
#include <string>
#include <map>


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
 class ini_parser;

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

	 bool RecordModeOn();

 private:
	 Config(const std::string& pathName);

 private:
	 static Config* m_instance;
	 ini_parser* parser_instance;
 };


#endif