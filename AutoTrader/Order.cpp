#include "stdafx.h"
#include "Order.h"


Order::Order()
{
}


Order::~Order()
{
}


Order::Order(const std::string& instrument, double refprice, ExchangeDirection direction, ExchangePriceType priceType, \
	TimeCondition timeCondition, VolumeCondition vCondition, ContingentCondition ctCondition)
{
	memset(&m_innerStruct, 0, sizeof(m_innerStruct));
	strcpy_s(m_innerStruct.InstrumentID, instrument.c_str());

	m_innerStruct.OrderPriceType = priceType;
	m_innerStruct.Direction = direction;
	//req.CombOffsetFlag[0] = kpp[0]; // comput TThostFtdcCombOffsetFlagType according current account balance
	m_innerStruct.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	m_innerStruct.LimitPrice = refprice;
	//m_innerStruct.VolumeTotalOriginal = vol; // compute Volume according current account available 
	
	m_innerStruct.TimeCondition = timeCondition;
	m_innerStruct.VolumeCondition = vCondition;
	m_innerStruct.MinVolume = 1;
	m_innerStruct.ContingentCondition = ctCondition;

	//TThostFtdcPriceType	StopPrice;  
	m_innerStruct.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	m_innerStruct.IsAutoSuspend = 0;
	m_innerStruct.UserForceClose = 0;
}

Order::Order(const std::string& instrument, double refprice, ExchangeDirection direction, OrderType type)
{
	memset(&m_innerStruct, 0, sizeof(m_innerStruct));
	strcpy_s(m_innerStruct.InstrumentID, instrument.c_str());
	//FAK 立即成交剩余指令自动撤销指令 (THOST_FTDC_OPT_LimitPrice THOST_FTDC_TC_IOC + THOST_FTDC_VC_AV)
	if (type == FAK){
		m_innerStruct.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
		m_innerStruct.Direction = direction;
		//m_innerStruct.CombOffsetFlag[0] = kpp[0]; // comput TThostFtdcCombOffsetFlagType according current account balance
		m_innerStruct.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
		m_innerStruct.LimitPrice = refprice;
		//m_innerStruct.VolumeTotalOriginal = vol; // compute Volume according current account available 

		m_innerStruct.TimeCondition = THOST_FTDC_TC_IOC;
		m_innerStruct.VolumeCondition = THOST_FTDC_VC_AV;
		m_innerStruct.MinVolume = 1;
		m_innerStruct.ContingentCondition = THOST_FTDC_CC_Immediately;

		//TThostFtdcPriceType	StopPrice;  
		m_innerStruct.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		m_innerStruct.IsAutoSuspend = 0;
		m_innerStruct.UserForceClose = 0;
	}
	//FAK 立即全部成交否则自动撤销指令 (THOST_FTDC_OPT_LimitPrice THOST_FTDC_TC_IOC + THOST_FTDC_VC_CV)
	else if (type == OrderType::FOK){
		m_innerStruct.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
		m_innerStruct.Direction = direction;
		//m_innerStruct.CombOffsetFlag[0] = kpp[0]; // comput TThostFtdcCombOffsetFlagType according current account balance
		m_innerStruct.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
		m_innerStruct.LimitPrice = refprice;
		//m_innerStruct.VolumeTotalOriginal = vol; // compute Volume according current account available 

		m_innerStruct.TimeCondition = THOST_FTDC_TC_IOC;
		m_innerStruct.VolumeCondition = THOST_FTDC_VC_CV;
		m_innerStruct.MinVolume = 1;
		m_innerStruct.ContingentCondition = THOST_FTDC_CC_Immediately;

		//TThostFtdcPriceType	StopPrice;  
		m_innerStruct.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		m_innerStruct.IsAutoSuspend = 0;
		m_innerStruct.UserForceClose = 0;
	}
}

bool Order::IsValid(){
	//for now, only support FAk
	if (m_innerStruct.OrderPriceType == THOST_FTDC_OPT_LimitPrice && \
		m_innerStruct.TimeCondition == THOST_FTDC_TC_IOC && \
		m_innerStruct.VolumeCondition == THOST_FTDC_VC_AV && \
		m_innerStruct.ContingentCondition == THOST_FTDC_CC_Immediately &&
		strcmp(m_innerStruct.InstrumentID, "") != 0)
		return true;

	return false;

}

void Order::SetIdentityInfo(const std::string& brokerId, const std::string& userId, const std::string& investorId, const std::string& ordRef){
	strcpy_s(m_innerStruct.BrokerID, sizeof(m_innerStruct.BrokerID), brokerId.c_str());
	strcpy_s(m_innerStruct.InvestorID, sizeof(m_innerStruct.InvestorID), investorId.c_str());

	strcpy_s(m_innerStruct.UserID, sizeof(m_innerStruct.UserID), userId.c_str());
	strcpy_s(m_innerStruct.OrderRef, sizeof(m_innerStruct.OrderRef), ordRef.c_str());
}