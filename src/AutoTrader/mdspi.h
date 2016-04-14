#ifndef MD_SPI_H_
#define MD_SPI_H_

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include "ThostFtdcMdApi.h"
#include "TickWrapper.h"
#include "KData.h"
#include <boost/lockfree/queue.hpp>

class DBWrapper;

class CtpMdSpi : public CThostFtdcMdSpi
{
	class MDThreadStateChangedHandler{
	public:
		MDThreadStateChangedHandler(CtpMdSpi* pMdUserSpi, const std::vector<std::string>& instrumentsStr, const std::string& brokerId, const std::string& userID, const std::string& pw)
			: m_MdUserSpiPtr(pMdUserSpi)
			, m_brokerId(brokerId)
			, m_userID(userID)
			, m_password(pw)
		{
			m_instruments.assign(instrumentsStr.begin(), instrumentsStr.end());
		}

		void OnFrontConnected(){
			m_MdUserSpiPtr->ReqUserLogin(const_cast<char*>(m_brokerId.c_str()), const_cast<char*>(m_userID.c_str()), const_cast<char*>(m_password.c_str()));
		}

		void OnLogined(){
			m_MdUserSpiPtr->SubscribeMarketData(m_instruments);
		}

	private:
		CtpMdSpi*                 m_MdUserSpiPtr;
		std::string               m_brokerId;
		std::string               m_userID;
		std::string               m_password;
		std::vector<std::string>  m_instruments;
	};

public:
	CtpMdSpi(CThostFtdcMdApi* api, const std::vector<std::string>& instruments, const std::string& brokerId, const std::string& userID, const std::string& pw);

	virtual ~CtpMdSpi();

	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);
		
	virtual void OnHeartBeatWarning(int nTimeLapse);

	virtual void OnFrontConnected();
	
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

public:
	bool ReqUserLogin(TThostFtdcBrokerIDType appId, TThostFtdcUserIDType userId, TThostFtdcPasswordType	passwd);
	void SubscribeMarketData(const std::vector<std::string>& instIdList);
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
	void SerializeToDb();
	void UpdateCachedTickData(const TickWrapper& tick); // OnReceiveTick
	std::vector<TickWrapper>& GetTickVec(const std::string& instrument);
	std::vector<TickWrapper>& GetTickVec60(const std::string& instrument);
	std::vector<KData>&       GetKDataVec(const std::string& instrument);

private:
  CThostFtdcMdApi* 													pUserApi;
  int              													m_requestId;
  MDThreadStateChangedHandler 										m_stateChangeHandler;
  std::map<std::string, std::vector<TickWrapper> >					m_TickMap;
  std::map<std::string, std::vector<TickWrapper> >              	m_TickMap60;
  std::map<std::string, std::vector<KData> >						m_KDataMap;

  std::shared_ptr<DBWrapper>										m_dbptr;  
};

#endif