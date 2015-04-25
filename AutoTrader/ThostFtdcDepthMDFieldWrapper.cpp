#include "stdafx.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "ThostFtdcUserApiStruct.h"
#include "DBWrapper.h"
#include <sstream>

CThostFtdcDepthMDFieldWrapper::CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p):
	m_innerPtr(p)
{
}


CThostFtdcDepthMDFieldWrapper::~CThostFtdcDepthMDFieldWrapper()
{
}

void CThostFtdcDepthMDFieldWrapper::serializeToDB() const {
	std::stringstream sql;
	sql << "INSERT INTO `test` (` ";
	sql << "Date" << "`,`";
	sql << "InstrumentID" << "`,`";
	sql << "ExchangeID" << "`,`";
	sql << "ExchangeInstID" << "`,`";
	sql << "LastPrice" << "`,`";
	sql << "PreSettlementPrice" << "`,`";
	sql << "PreClosePrice" << "`,`";
	sql << "PreOpenInterest" << "`,`";
	sql << "OpenPrice" << "`,`";
	sql << "HighestPrice" << "`,`";
	sql << "LowestPrice" << "`,`";
	sql << "Volume" << "`,`";
	sql << "Turnover" << "`,`";
	sql << "OpenPrice" << "`,`";
	sql << "OpenInterest" << "`,`";
	sql << "ClosePrice" << "`,`";
	sql << "SettlementPrice" << "`,`";
	sql << "UpperLimitPrice" << "`,`";
	sql << "LowerLimitPrice" << "`,`";
	sql << "PreDelta" << "`,`";
	sql << "CurrDelta"<< "`,`";
	sql << "UpdateTime"<< "`,`";
	sql << "UpdateMillisec" << "`,`";
	sql << "BidPrice1" << "`,`";
	sql << "BidVolume1" << "`,`";
	sql << "AskPrice1" << "`,`";
	sql << "AskVolume1" << "`,`";
	sql << "BidPrice2" << "`,`";
	sql << "BidVolume2" << "`,`";
	sql << "AskPrice2" << "`,`";
	sql << "AskVolume2" << "`,`";
	sql << "BidPrice3" << "`,`";
	sql << "BidVolume3" << "`,`";
	sql << "AskPrice3" << "`,`";
	sql << "AskVolume3" << "`,`";
	sql << "BidPrice4" << "`,`";
	sql << "BidVolume4" << "`,`";
	sql << "AskPrice4" << "`,`";
	sql << "AskVolume4" << "`,`";
	sql << "BidPrice5" << "`,`";
	sql << "BidVolume5" << "`,`";
	sql << "AskPrice5" << "`,`";
	sql << "AskVolume5" << "`,`";
	sql << "AveragePrice" << "`,`";
	sql << "ActionDay" << "`,`";
	sql << ") VALUES(";
	sql << m_innerPtr->TradingDay << ", ";
	sql << m_innerPtr->InstrumentID << ", ";
	sql << m_innerPtr->ExchangeID << ", ";
	sql << m_innerPtr->ExchangeInstID << ", ";
	sql << m_innerPtr->LastPrice << ", ";
	sql << m_innerPtr->PreSettlementPrice << ", ";
	sql << m_innerPtr->PreClosePrice << ", ";
	sql << m_innerPtr->PreOpenInterest << ", ";
	sql << m_innerPtr->OpenPrice << ", ";
	sql << m_innerPtr->HighestPrice << ", ";
	sql << m_innerPtr->LowestPrice << ", ";
	sql << m_innerPtr->Volume << ", ";
	sql << m_innerPtr->Turnover << ", ";
	sql << m_innerPtr->OpenInterest << ", ";
	sql << m_innerPtr->ClosePrice << ", ";
	sql << m_innerPtr->SettlementPrice << ", ";
	sql << m_innerPtr->UpperLimitPrice << ", ";
	sql << m_innerPtr->LowerLimitPrice << ", ";
	sql << m_innerPtr->PreDelta << ", ";
	sql << m_innerPtr->CurrDelta << ", ";
	sql << m_innerPtr->UpdateTime << ", ";
	sql << m_innerPtr->UpdateMillisec << ", ";
	sql << m_innerPtr->BidPrice1 << ", ";
	sql << m_innerPtr->BidVolume1 << ", ";
	sql << m_innerPtr->AskPrice1 << ", ";
	sql << m_innerPtr->AskVolume1 << ", ";
	sql << m_innerPtr->BidPrice2 << ", ";
	sql << m_innerPtr->BidVolume2 << ", ";
	sql << m_innerPtr->AskPrice2 << ", ";
	sql << m_innerPtr->AskVolume2 << ", ";
	sql << m_innerPtr->BidPrice3 << ", ";
	sql << m_innerPtr->BidVolume3 << ", ";
	sql << m_innerPtr->AskPrice3 << ", ";
	sql << m_innerPtr->AskVolume3 << ", ";
	sql << m_innerPtr->BidPrice4 << ", ";
	sql << m_innerPtr->BidVolume4 << ", ";
	sql << m_innerPtr->AskPrice4 << ", ";
	sql << m_innerPtr->AskVolume4 << ", ";
	sql << m_innerPtr->BidPrice5 << ", ";
	sql << m_innerPtr->BidVolume5 << ", ";
	sql << m_innerPtr->AskPrice5 << ", ";
	sql << m_innerPtr->AskVolume5 << ", ";
	sql << m_innerPtr->AveragePrice << ", ";
	sql << m_innerPtr->ActionDay << ")";
	//"INSERT INTO `test` (`name`) VALUES (1234) 
	DBWrapper::GetDBWrapper().ExecuteNoResult(sql.str());
}
