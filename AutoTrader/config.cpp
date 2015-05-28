#include "stdafx.h"
#include "config.h"
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