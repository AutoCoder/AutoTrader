#include <fstream>
#include "ConfigV2.h"
#include "json/json.h"

ConfigV2* ConfigV2::m_instance = NULL;

ConfigV2::ConfigV2(const std::string& pathName)
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

ConfigV2::~ConfigV2(){

}

ConfigV2* ConfigV2::Instance()
{
	if (!m_instance){
		m_instance = new ConfigV2("config_v2.json");
	}

	return m_instance;
}

std::string ConfigV2::CtpMdFront() const{
	return m_ctp_mdfront;
}

std::string ConfigV2::CtpTradeFront() const{
	return m_ctp_tradefront;
}

std::string ConfigV2::DBHost() const{
	return m_db_host;
}

int ConfigV2::DBPort(){
	return m_db_port;
}

std::string ConfigV2::DBName() const{
	return m_db_name;
}

std::string ConfigV2::DBUser() const{
	return m_db_user;
}

std::string ConfigV2::DBPassword() const{
	return m_db_password;
}


std::string ConfigV2::ProductName() const
{
	return m_db_productName;
}

std::string ConfigV2::DefaultCtpBrokerID() const{
	return m_ctp_brokerId;
}
std::string ConfigV2::DefaultCtpUserID() const{
	return m_ctp_userId;
}
std::string ConfigV2::DefaultCtpPassword() const{
	return m_ctp_password;
}