#include "stdafx.h"
#include "config.h"
#include <fstream>
#include "Strategy.h"
#include "json/json.h"
#ifdef CONF_INI
#include "ini_parser.h"
Config* Config::m_instance = NULL;

Config::Config(const std::string& pathName)
	:parser_instance(new ini_parser(pathName))
{
}

Config::~Config(){
	delete parser_instance;
}

Config* Config::Instance()
{
	if (!m_instance){
		m_instance = new Config("config.ini");
	}

	return m_instance;
}

std::string Config::CtpBrokerID(){
	return parser_instance->get_string("BrokerID", "CtpAccount");
}

std::string Config::CtpUserID(){
	return parser_instance->get_string("UserID", "CtpAccount");
}

std::string Config::CtpPassword(){
	return parser_instance->get_string("Password", "CtpAccount");
}

std::string Config::CtpMdFront(){
	return parser_instance->get_string("MdFront", "CtpServer");
}

std::string Config::CtpTradeFront(){
	return parser_instance->get_string("TradeFront", "CtpServer");
}

std::string Config::CtpInstrumentIDs(){
	return parser_instance->get_string("InstrumentIDs", "Instruments");
}

std::string Config::CtpStrategy(const std::string& instrumentID){
	return parser_instance->get_string(instrumentID, "Strategy");
}

std::string Config::DBHost(){
	return parser_instance->get_string("Host", "DataBase");
}

int Config::DBPort(){
	return parser_instance->get_int("Port", "DataBase");
}

std::string Config::DBName(){
	return parser_instance->get_string("DBName", "DataBase");
}

std::string Config::DBUser(){
	return parser_instance->get_string("User", "DataBase");
}

std::string Config::DBPassword(){
	return parser_instance->get_string("Password", "DataBase");
}

bool Config::RecordModeOn(){
	return parser_instance->get_bool("RecordMode", "Arguments");
}

std::string Config::ProductName()
{
	return parser_instance->get_string("Name", "ProductInfo");
}

#endif



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
		if (!root["CtpAccount"].isNull()){
			m_ctp_brokerId = root["CtpAccount"]["BrokerID"].asString();
			m_ctp_userId = root["CtpAccount"]["UserID"].asString();
			m_ctp_password = root["CtpAccount"]["Password"].asString();
		}
		
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

		if (!root["Strategies"].isNull()){
			int st_size = root["Strategies"].size();

			for (int i = 0; i < st_size; ++i)
			{
				StrategyMetaData st;
				st.name = root["Strategies"][i]["Name"].asString();
				st.short_ma = root["Strategies"][i]["ShortMA"].asInt();
				st.long_ma = root["Strategies"][i]["LongMA"].asInt();
				m_strategies.push_back(st);
			}
		}

		if (!root["Instruments"].isNull()){
			m_instruments = root["Instruments"].asString();
		}

		if (!root["Arguments"].isNull()){
			m_recodeMode = root["Arguments"]["RecordMode"].asBool();
		}

		if (!root["StrategyMap"].isNull()){
			m_strategymap["rb1510"] = root["StrategyMap"]["rb1510"].asString();
			m_strategymap["rb1511"] = root["StrategyMap"]["rb1511"].asString();
		}
	}
	is.close();
}

Config::~Config(){

}

Config* Config::Instance()
{
	if (!m_instance){
		m_instance = new Config("config.json");
	}

	return m_instance;
}


std::string Config::CtpBrokerID(){
	return m_ctp_brokerId;
}

std::string Config::CtpUserID(){
	return m_ctp_userId;
}

std::string Config::CtpPassword(){
	return m_ctp_password;
}

std::string Config::CtpMdFront(){
	return m_ctp_mdfront;
}

std::string Config::CtpTradeFront(){
	return m_ctp_tradefront;
}

std::string Config::CtpInstrumentIDs(){
	return m_instruments;
}

std::string Config::CtpStrategy(const std::string& instrumentID){
	if (m_strategymap.find(instrumentID) != m_strategymap.end()){
		return m_strategymap[instrumentID];
	}
	else{
		return "";
	}
}

std::string Config::DBHost(){
	return m_db_host;
}

int Config::DBPort(){
	return m_db_port;
}

std::string Config::DBName(){
	return m_db_name;
}

std::string Config::DBUser(){
	return m_db_user;
}

std::string Config::DBPassword(){
	return m_db_password;
}

bool Config::RecordModeOn(){
	return m_recodeMode;
}

std::string Config::ProductName()
{
	return m_db_productName;
}

std::vector<StrategyMetaData> Config::StrategySet(void){
	return m_strategies;
}