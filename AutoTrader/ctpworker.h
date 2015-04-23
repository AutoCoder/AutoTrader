#pragma once
#include <vector>
#include <string>
#include "ThostFtdcMdApi.h"
#include "ThostFtdcTraderApi.h"

class ctpworker
{
public:
	ctpworker();
	~ctpworker();

	virtual bool Setup() = 0;

};

class CtpMdSpi;
class mdctpworker : public ctpworker{
public:
	mdctpworker(const std::string& frontAddress, const std::string&  brokerId, const std::string&  userID, const std::string&  password);
	~mdctpworker();
	virtual bool Setup();

	bool login();
	bool subscribeInstrument(const std::string& instrumentIds); // "rb1501,SR1501"
	void join();

private:
	std::string m_frontAddress;
	TThostFtdcBrokerIDType	m_brokerId;
	TThostFtdcUserIDType	m_userID;
	TThostFtdcPasswordType m_password;
	CThostFtdcMdApi* m_pUserApi;
	CtpMdSpi* m_pMdSpi;
};

class CtpTraderSpi;
class tradectpworker : public ctpworker{
public:
	tradectpworker(const std::string& frontAddress, const std::string&  brokerId, const std::string&  userID, const std::string&  password);
	~tradectpworker();
	virtual bool Setup();

	bool login();
	bool subscribeInstrument(const std::string& instrumentIds); // "rb1501,SR1501"
	void join();

	void DoTrade() {};

private:
	std::string m_frontAddress;
	TThostFtdcBrokerIDType	m_brokerId;
	TThostFtdcUserIDType	m_userID;
	TThostFtdcPasswordType m_password;
	CThostFtdcTraderApi* m_pUserApi;
	CtpTraderSpi* m_pTradeSpi;
};