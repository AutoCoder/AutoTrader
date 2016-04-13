#include "Order.h"
#include <assert.h>
#include "crossplatform.h"

Order::Order()
{
	memset(&m_innerStruct, 0, sizeof(m_innerStruct));
}


Order::~Order()
{
}


Order::Order(const std::string& instrument, TThostFtdcPriceType refprice,
	TThostFtdcVolumeType vol,
	TThostFtdcDirectionType direction,
	TThostFtdcCombOffsetFlagType kpp)
{
	memset(&m_innerStruct, 0, sizeof(m_innerStruct));
	STRCPY(m_innerStruct.InstrumentID, instrument.c_str());

	if (std::abs(refprice) < std::numeric_limits<double>::epsilon()){
		m_innerStruct.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
		m_innerStruct.TimeCondition = THOST_FTDC_TC_IOC;
	}
	else{
		m_innerStruct.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
		m_innerStruct.TimeCondition = THOST_FTDC_TC_IOC;
	}

	m_innerStruct.Direction = direction;
	STRCPY(m_innerStruct.CombHedgeFlag, kpp);
	m_innerStruct.LimitPrice = refprice;
	m_innerStruct.VolumeTotalOriginal = vol;
	
	m_innerStruct.VolumeCondition = THOST_FTDC_VC_AV;
	m_innerStruct.MinVolume = 1;
	m_innerStruct.ContingentCondition = THOST_FTDC_CC_Immediately;

	//TThostFtdcPriceType	StopPrice;  
	m_innerStruct.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	m_innerStruct.IsAutoSuspend = 0;
	m_innerStruct.UserForceClose = 0;
}
//
//Order::Order(const std::string& instrument, double refprice, ExchangeDirection direction, OrderType type)
//{
//	memset(&m_innerStruct, 0, sizeof(m_innerStruct));
//	STRCPY(m_innerStruct.InstrumentID, instrument.c_str());
//	//FAK 立即成交剩余指令自动撤销指令 (THOST_FTDC_OPT_LimitPrice THOST_FTDC_TC_IOC + THOST_FTDC_VC_AV)
//	if (type == FAK){
//		m_innerStruct.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
//		m_innerStruct.Direction = direction;
//		//m_innerStruct.CombOffsetFlag[0] = kpp[0]; // comput TThostFtdcCombOffsetFlagType according current account balance
//		m_innerStruct.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
//		m_innerStruct.LimitPrice = refprice;
//		//m_innerStruct.VolumeTotalOriginal = vol; // compute Volume according current account available 
//
//		m_innerStruct.TimeCondition = THOST_FTDC_TC_IOC;
//		m_innerStruct.VolumeCondition = THOST_FTDC_VC_AV;
//		m_innerStruct.MinVolume = 1;
//		m_innerStruct.ContingentCondition = THOST_FTDC_CC_Immediately;
//
//		//TThostFtdcPriceType	StopPrice;  
//		m_innerStruct.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
//		m_innerStruct.IsAutoSuspend = 0;
//		m_innerStruct.UserForceClose = 0;
//	}
//	//FAK 立即全部成交否则自动撤销指令 (THOST_FTDC_OPT_LimitPrice THOST_FTDC_TC_IOC + THOST_FTDC_VC_CV)
//	else if (type == OrderType::FOK){
//		m_innerStruct.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
//		m_innerStruct.Direction = direction;
//		//m_innerStruct.CombOffsetFlag[0] = kpp[0]; // comput TThostFtdcCombOffsetFlagType according current account balance
//		m_innerStruct.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
//		m_innerStruct.LimitPrice = refprice;
//		//m_innerStruct.VolumeTotalOriginal = vol; // compute Volume according current account available 
//
//		m_innerStruct.TimeCondition = THOST_FTDC_TC_IOC;
//		m_innerStruct.VolumeCondition = THOST_FTDC_VC_CV;
//		m_innerStruct.MinVolume = 1;
//		m_innerStruct.ContingentCondition = THOST_FTDC_CC_Immediately;
//
//		//TThostFtdcPriceType	StopPrice;  
//		m_innerStruct.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
//		m_innerStruct.IsAutoSuspend = 0;
//		m_innerStruct.UserForceClose = 0;
//	}
//}

bool Order::IsValid(){
	//for now, only support LimitPriceFOKOrder
	if (m_innerStruct.OrderPriceType == THOST_FTDC_OPT_LimitPrice && \
		m_innerStruct.TimeCondition == THOST_FTDC_TC_GFD && \
		m_innerStruct.VolumeCondition == THOST_FTDC_VC_AV && \
		strcmp(m_innerStruct.InstrumentID, "") != 0)
		return true;

	return false;  

}

void Order::SetIdentityInfo(const std::string& brokerId, const std::string& userId, const std::string& investorId, const std::string& ordRef){
	STRCPY(m_innerStruct.BrokerID, brokerId.c_str());
	STRCPY(m_innerStruct.InvestorID, investorId.c_str());

	STRCPY(m_innerStruct.UserID, userId.c_str());
	STRCPY(m_innerStruct.OrderRef, ordRef.c_str());
}

void Order::SetInstrumentId(const std::string& in){
	STRCPY(m_innerStruct.InstrumentID, in.c_str());
}

void Order::SetOrderType(OrderType type){
	m_type = type;
	m_innerStruct.MinVolume = 1;
	m_innerStruct.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	m_innerStruct.IsAutoSuspend = 0;
	m_innerStruct.UserForceClose = 0;
	m_innerStruct.CombHedgeFlag[0] = THOST_FTDC_BHF_Speculation;

	switch (type){
		case LimitPriceFOKOrder:
		{
			m_innerStruct.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
			m_innerStruct.TimeCondition = THOST_FTDC_TC_GFD;
			m_innerStruct.VolumeCondition = THOST_FTDC_VC_AV;
			m_innerStruct.ContingentCondition = THOST_FTDC_CC_Immediately;
			break;
		}
		case LimitPriceFAKOrder:
		{
			m_innerStruct.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
			m_innerStruct.TimeCondition = THOST_FTDC_TC_GFD;
			m_innerStruct.VolumeCondition = THOST_FTDC_VC_CV;
			m_innerStruct.ContingentCondition = THOST_FTDC_CC_Immediately;
			break;
		}
		case AnyPriceOrder:
		{
			m_innerStruct.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
			m_innerStruct.LimitPrice = 0;
			m_innerStruct.TimeCondition = THOST_FTDC_TC_IOC;
			break;
		}
		case TriggerOrder:
		{
			m_innerStruct.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
			m_innerStruct.ContingentCondition = THOST_FTDC_CC_Immediately;
			break;
		}
		default:
		{
			assert(false);
			break;
		}
	}
}


void Order::SetRefExchangePrice(double price){
	switch (m_type){
		case LimitPriceFOKOrder:
		case LimitPriceFAKOrder:
			m_innerStruct.LimitPrice = price;
			break;
		case AnyPriceOrder:
			{
				SYNC_PRINT << "[Order class]: For AnyPriceOrder, the limitprice should be always 0";
				assert(false);
				m_innerStruct.LimitPrice = 0;
			}
			break;
		case TriggerOrder:
			m_innerStruct.StopPrice = price;
			break;
		default:
		{
			SYNC_PRINT << "[Order class]: Invalid Order type.";
			assert(false);
		}
		break;
	}
}