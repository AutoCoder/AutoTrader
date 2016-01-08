#include "stdafx.h"
#include "ConfigV2.h"
#include <fstream>
#include "json/json.h"
#include "Account.h"
#include "AccountMgr.h"

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

		if (!root["AccountList"].isNull()){
			int ac_size = root["AccountList"].size();
			for (int i = 0; i < ac_size; ++i){

			}
		}

		if (!root["StrategyList"].isNull()){
			int st_size = root["StrategyList"].size();

			for (int i = 0; i < st_size; ++i)
			{
				m_strategies.push_back(root["Strategies"][i].asString());
			}
		}

		if (!root["AccountList"].isNull()){
			auto acnode = root["AccountList"];
			int ac_size = root["AccountList"].size();
			
			for (int i = 0; i < ac_size; ++i)
			{
				auto& item = root["AccountList"][i];
				std::shared_ptr<Account> account = std::make_shared<Account>(item["BrokerID"].asString(), item["UserID"].asString(), item["Password"].asString());

				auto& instruList = item["Instruments"];
				for (int i = 0; i < instruList.size(); ++i){
					account->AddInstrument(instruList[i].asString());
					m_instruments.insert(instruList[i].asString());
				}
				
				auto& strategyList = item["Strategies"];
				for (int i = 0; i < strategyList.size(); ++i){
					account->AddStrategy(strategyList[i].asInt());
				}

				auto& pcList = item["PositionControl"];
				for (int i = 0; i < pcList.size(); ++i){
					account->AddPositionControl(pcList[i].asInt());
				}

				AccountMgr::getInstance()->AddAccount(account);

			}
		}

	}
	is.close();
}

ConfigV2::~ConfigV2(){

}

ConfigV2* ConfigV2::Instance()
{
	if (!m_instance){
		m_instance = new ConfigV2("config.json");
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

std::string ConfigV2::GetStrategy(size_t idx) const
{
	if (idx <= m_strategies.size())
		return m_strategies.at(idx);
	else
		return "";
}
