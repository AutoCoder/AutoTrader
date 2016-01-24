#include <fstream>
#include "Config.h"
#include "json/json.h"

Config* Config::m_instance = NULL;

Config::Config(const std::string& pathName)
:m_filepath(pathName)
{
	Json::Reader reader;

	Json::Value root;

	std::ifstream is;
	is.open(m_filepath, std::ios::binary);
	if (reader.parse(is, root))
	{
		std::string code;

		if (!root["CtpServer"].isNull()){
			m_ctp_mdfront = root["CtpServer"]["MdFront"].asString();
			m_ctp_tradefront = root["CtpServer"]["TradeFront"].asString();
			m_ctp_riskfront = root["CtpServer"]["RiskFront"].asString();
		}

		if (!root["DataBase"].isNull()){
			m_db_host = root["DataBase"]["Host"].asString();
			m_db_port = root["DataBase"]["Port"].asInt();
			m_db_name = root["DataBase"]["DBName"].asString();
			m_db_user = root["DataBase"]["User"].asString();
			m_db_password = root["DataBase"]["Password"].asString();
		}

		if (!root["ProductInfo"].isNull()){
			m_db_productName = root["ProductInfo"]["Name"].asString();
		}


		if (!root["MD_Accout"].isNull()){
			auto node = root["MD_Accout"];
			m_ctp_brokerId = node["BrokerID"].asString();
			m_ctp_userId = node["UserID"].asString();
			m_ctp_password = node["Password"].asString();
		}

	}
	is.close();
}

Config::~Config(){

}

Config* Config::Instance()
{
	if (!m_instance){
		m_instance = new Config("config_v2.json");
	}

	return m_instance;
}

std::string Config::CtpMdFront() const{
	return m_ctp_mdfront;
}

std::string Config::CtpTradeFront() const{
	return m_ctp_tradefront;
}

std::string Config::DBHost() const{
	return m_db_host;
}

int Config::DBPort(){
	return m_db_port;
}

std::string Config::DBName() const{
	return m_db_name;
}

std::string Config::DBUser() const{
	return m_db_user;
}

std::string Config::DBPassword() const{
	return m_db_password;
}


std::string Config::ProductName() const
{
	return m_db_productName;
}

std::string Config::DefaultCtpBrokerID() const{
	return m_ctp_brokerId;
}
std::string Config::DefaultCtpUserID() const{
	return m_ctp_userId;
}
std::string Config::DefaultCtpPassword() const{
	return m_ctp_password;
}