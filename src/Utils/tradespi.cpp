#include "tradespi.h"
#include "spdlog/spdlog.h"


CtpTradeSpi::CtpTradeSpi(CThostFtdcTraderApi* p, const char * brokerID, const char* userID, const char* password, const char* prodname)
	: m_frontID(-1)
	, m_sessionID(-1)
	, m_querying(true)
	, m_stateChangeHandler(this)
	, m_requestId(0)
	, pUserApi(p)
{
	STRCPY(m_brokerID, brokerID);
	STRCPY(m_userID, userID);
	STRCPY(m_password, password);
	STRCPY(m_productName, prodname);
}

CtpTradeSpi::~CtpTradeSpi(){}



void CtpTradeSpi::OnFrontConnected(){
	SYNC_PRINT << "[Trade] Response | connected...";
	m_stateChangeHandler.OnFrontConnected();
}

void CtpTradeSpi::ReqUserLogin(){
	CThostFtdcReqUserLoginField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.UserID, m_userID);
	STRCPY(req.Password, m_password);
	STRCPY(req.UserProductInfo, m_productName);
	int ret = pUserApi->ReqUserLogin(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Login ..." << ((ret == 0) ? "Success" : "Fail");
}

void CtpTradeSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin) {

		m_frontID = pRspUserLogin->FrontID;
		m_sessionID = pRspUserLogin->SessionID;

		int nextOrderRef = atoi(pRspUserLogin->MaxOrderRef);
		SPRINTF(m_orderRef, "%d", ++nextOrderRef);

		SYNC_PRINT << "[Trade] Response | login success...Current Date:" << pRspUserLogin->TradingDay << "| FrontID: " << m_frontID << "| SessionID: " << m_sessionID;

		m_stateChangeHandler.OnLogined();
	}
}

void CtpTradeSpi::ReqSettlementInfoConfirm(){
	CThostFtdcSettlementInfoConfirmField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	int ret = pUserApi->ReqSettlementInfoConfirm(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Send Settlement Confirm..." << ((ret == 0) ? "Success" : "Fail");
}

void CtpTradeSpi::OnRspSettlementInfoConfirm( CThostFtdcSettlementInfoConfirmField  *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast){
	if (!IsErrorRspInfo(pRspInfo) && pSettlementInfoConfirm){
		SYNC_PRINT << "[Trade] Response | Settlement Confirm Success..." 
			<< pSettlementInfoConfirm->InvestorID
			<< "...<" << pSettlementInfoConfirm->ConfirmDate
			<< " " << pSettlementInfoConfirm->ConfirmTime << ">...Confirm";
	}

	m_stateChangeHandler.OnConfirmedSettlementInfo();
}



void CtpTradeSpi::ReqQryInstrument_all(){
	CThostFtdcQryInstrumentField req;
	memset(&req, 0, sizeof(req));
	int ret = pUserApi->ReqQryInstrument(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query Instrument(All)..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}



void CtpTradeSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrument)
	{
		if (((pInstrument->ProductClass == THOST_FTDC_PC_Futures) || (pInstrument->ProductClass == THOST_FTDC_PC_Options) ||
			(pInstrument->ProductClass == THOST_FTDC_PC_SpotOption) /*||(pInstrument->ProductClass ==THOST_FTDC_PC_Combination)*/) /*&&
			 (pInstrument->IsTrading)*/)
		{
			Instrument::Information info(*pInstrument);

			std::string instru_key(pInstrument->InstrumentID);
			InstrumentManager.Add(instru_key, info);
		}
	}
	else
	{
		if (pRspInfo)
		{
			SYNC_PRINT << "[Trade] Failed to query instrument info, error:" << pRspInfo->ErrorMsg;
		}
	}

	if (bIsLast)
	{
		//SYNC_PRINT << "[Trade] All Instruments in Position:" << //todo: return all position's instruments;
		SYNC_PRINT << "[Trade] All Instruments Queried:" << InstrumentManager.AllInstruments();

		m_stateChangeHandler.OnLastRspQryInstrument();
	}
}

bool CtpTradeSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
	bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
	if (ret && !std::string(pRspInfo->ErrorMsg).empty()){
		SYNC_PRINT << "[Trade] Response | " << pRspInfo->ErrorMsg;
	}
	return ret;
}


///请求查询合约保证金率
void CtpTradeSpi::ReqQryInstrumentMarginRate(const std::string& instrumentId)
{
	CThostFtdcQryInstrumentMarginRateField req;
	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	STRCPY(req.InstrumentID, instrumentId.c_str());

	req.HedgeFlag = THOST_FTDC_HF_Speculation;

	int ret = pUserApi->ReqQryInstrumentMarginRate(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query MarginRate..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}

//响应查询合约保证金率
void CtpTradeSpi::OnRspQryInstrumentMarginRate(CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrumentMarginRate)
	{
		InstrumentManager.SetMarginRate(pInstrumentMarginRate->InstrumentID, *pInstrumentMarginRate);
	}
	else if (pInstrumentMarginRate)
	{
		SYNC_PRINT << "[Trade] Reponse | failed to obtain the margin rate field for " << pInstrumentMarginRate->InstrumentID;
	}

	NotifyQueryEnd();
}

///请求查询合约手续费率
void CtpTradeSpi::ReqQryInstrumentCommissionRate(const std::string& instrumentId)
{
	CThostFtdcQryInstrumentCommissionRateField req;

	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	STRCPY(req.InstrumentID, instrumentId.c_str());

	int ret = pUserApi->ReqQryInstrumentCommissionRate(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query CommissionRate..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}

///响应查询合约手续费率
void CtpTradeSpi::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pInstrumentCommissionRate)
	{
		InstrumentManager.SetCommissionRate(pInstrumentCommissionRate->InstrumentID, *pInstrumentCommissionRate);
	}
	else if (pInstrumentCommissionRate)
	{
		SYNC_PRINT << "[Trade] Reponse | failed to obtain the commission rate field for " << pInstrumentCommissionRate->InstrumentID;
	}

	NotifyQueryEnd();
}

///请求查询期权交易成本
void CtpTradeSpi::ReqQryOptionInstrTradeCost(const std::string& instrumentId)
{
	CThostFtdcQryOptionInstrTradeCostField req;

	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	STRCPY(req.InstrumentID, instrumentId.c_str());

	req.HedgeFlag = THOST_FTDC_HF_Speculation;
	int ret = pUserApi->ReqQryOptionInstrTradeCost(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query OptionInstrTradeCost..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}

///响应查询期权交易成本
void CtpTradeSpi::OnRspQryOptionInstrTradeCost(CThostFtdcOptionInstrTradeCostField *pOptionInstrTradeCost, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pOptionInstrTradeCost)
	{
		//todo: store the Option InstrTradeCost
	}
	else
	{
		
	}
	if (bIsLast){
	}
}

///请求查询期权合约手续费
void CtpTradeSpi::ReqQryOptionInstrCommRate(const std::string& instrumentId)
{
	CThostFtdcQryOptionInstrCommRateField req;

	memset(&req, 0, sizeof(req));
	STRCPY(req.BrokerID, m_brokerID);
	STRCPY(req.InvestorID, m_userID);
	STRCPY(req.InstrumentID, instrumentId.c_str());

	int ret = pUserApi->ReqQryOptionInstrCommRate(&req, ++m_requestId);
	SYNC_PRINT << "[Trade] Request | Query OptionInstrCommRate..." << ((ret == 0) ? "Success" : "Fail") << " ret:" << ret;
}

///响应查询期权合约手续费
void CtpTradeSpi::OnRspQryOptionInstrCommRate(CThostFtdcOptionInstrCommRateField *pOptionInstrCommRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pOptionInstrCommRate)
	{
		//todo: store the Option OptionInstrCommRate
	}
	else
	{
		
	}

	if (bIsLast){

	}
}