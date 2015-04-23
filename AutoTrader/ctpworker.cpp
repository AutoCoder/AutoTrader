#include "stdafx.h"
#include "ctpworker.h"

#include "ThostFtdcTraderApi.h"
#include "mdspi.h"
#include "traderspi.h"


ctpworker::ctpworker()
{
}


ctpworker::~ctpworker()
{
}


mdctpworker::mdctpworker(const std::string& frontAddress, const std::string&  brokerId, const std::string&  userID, const std::string&  password)
	: m_frontAddress(frontAddress.c_str())
{
	strcpy_s(m_brokerId, brokerId.c_str());
	strcpy_s(m_userID, userID.c_str());
	strcpy_s(m_password, password.c_str());
}

mdctpworker::~mdctpworker()
{}

bool mdctpworker::Setup(){
	m_pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
	m_pMdSpi = new CtpMdSpi(m_pUserApi);
	m_pUserApi->RegisterSpi(m_pMdSpi);
	m_pUserApi->RegisterFront(const_cast<char*>(m_frontAddress.c_str()));

	m_pUserApi->Init();
	//ShowMdCommand(pUserSpi, true);
	return true;
}

bool mdctpworker::login()
{
	return m_pMdSpi->ReqUserLogin(m_brokerId, m_userID, m_password);
}

bool mdctpworker::subscribeInstrument(const std::string& instrumentIds)
{
	m_pMdSpi->SubscribeMarketData(const_cast<char*>(instrumentIds.c_str()));
	return true;
}

void mdctpworker::join(){
	m_pUserApi->Join();
}






tradectpworker::tradectpworker(const std::string& frontAddress, const std::string&  brokerId, const std::string&  userID, const std::string&  password)
	: m_frontAddress(frontAddress.c_str())
{
	strcpy_s(m_brokerId, brokerId.c_str());
	strcpy_s(m_userID, userID.c_str());
	strcpy_s(m_password, password.c_str());
}

tradectpworker::~tradectpworker()
{

}

bool tradectpworker::Setup(){
	m_pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
	m_pTradeSpi = new CtpTraderSpi(m_pUserApi);
	m_pUserApi->RegisterSpi((CThostFtdcTraderSpi*)m_pUserApi);
	m_pUserApi->SubscribePublicTopic(THOST_TERT_RESTART);
	m_pUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);
	m_pUserApi->RegisterFront(const_cast<char*>(m_frontAddress.c_str()));

	m_pUserApi->Init();
	return true;
}

bool tradectpworker::login()
{
	return false;
}

void tradectpworker::join()
{
	m_pUserApi->Join();
}