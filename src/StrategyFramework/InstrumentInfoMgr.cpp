#include <assert.h>
#include <fstream>
#include "json/json.h"

#include "stdafx.h"
#include "crossplatform.h"
#include "InstrumentInfoMgr.h"

namespace Instrument{

	static const char* FactorSettingFilePath = "InstrumentInfo.json";

	//Three Group
	static const char* kBasicInfo = "BasicInfo";
	static const char* kMarginInfo = "MarginInfo";
	static const char* kCommissionInfo = "CommissionInfo";

	static const char* kInstrumentID = "InstrumentID";
	static const char* kExchangeID = "ExchangeID";
	static const char* kDeliveryYear = "DeliveryYear";
	static const char* kDeliveryMonth = "DeliveryMonth";
	static const char* kVolumeMultiple = "VolumeMultiple";
	static const char* kOpenDate = "OpenDate";
	static const char* kExpireDate = "ExpireDate";
	static const char* kIsTrading = "IsTrading";
	static const char* kPositionType = "PositionType";

	//margin field
	static const char* kBrokerID = "BrokerID";
	static const char* kInvestorID = "InvestorID";
	static const char* kHedgeFlag = "HedgeFlag";
	static const char* kLongMarginRatioByMoney = "LongMarginRatioByMoney";
	static const char* kLongMarginRatioByVolume = "LongMarginRatioByVolume";
	static const char* kShortMarginRatioByMoney = "ShortMarginRatioByMoney";
	static const char* kShortMarginRatioByVolume = "ShortMarginRatioByVolume";

	//commission field
	static const char* kOpenRatioByMoney = "OpenRatioByMoney";
	static const char* kOpenRatioByVolume = "OpenRatioByVolume";
	static const char* kCloseRatioByMoney = "CloseRatioByMoney";
	static const char* kCloseRatioByVolume = "CloseRatioByVolume";
	static const char* kCloseTodayRatioByMoney = "CloseTodayRatioByMoney";
	static const char* kCloseTodayRatioByVolume = "CloseTodayRatioByVolume";

	InformationMgr& GetManager(){
		static InformationMgr mgr;
		return mgr;
	}

	

	InformationMgr::InformationMgr()
	{
		m_InfoDict.clear();
		unserialize();
	}


	InformationMgr::~InformationMgr()
	{
	}

	const Information& InformationMgr::Get(const std::string& instrumentID) const{
		const std::string& prodID = CommonUtils::InstrumentIDToProductID(instrumentID);
		assert (m_InfoDict.find(prodID) != m_InfoDict.end());
		return m_InfoDict.at(prodID);
	}

	int InformationMgr::VolumeMultiple(const std::string& instrumentID) const{
		const std::string& prodID = CommonUtils::InstrumentIDToProductID(instrumentID);
		assert (m_InfoDict.find(prodID) != m_InfoDict.end());
		return m_InfoDict.at(prodID).InstruField.VolumeMultiple;		
	}

	void InformationMgr::Add(const std::string& instrumentID, const Information& info){
		assert(!instrumentID.empty());
		m_InfoVec.push_back(instrumentID);
		const std::string& prodID = CommonUtils::InstrumentIDToProductID(instrumentID);
		if (m_InfoDict.find(prodID) == m_InfoDict.end()){
			m_InfoDict.insert(std::make_pair(prodID, info));

		}
	}

	bool InformationMgr::SetMarginRate(const std::string& instrumentID, const CThostFtdcInstrumentMarginRateField& mgrRate){
		assert(!instrumentID.empty());
		const std::string& prodID = CommonUtils::InstrumentIDToProductID(instrumentID);
		bool prod_existed = (m_InfoDict.find(prodID) != m_InfoDict.end());
		m_InfoDict[prodID].MgrRateField = mgrRate;
		return prod_existed;
	}

	bool InformationMgr::SetCommissionRate(const std::string& instrumentID, const CThostFtdcInstrumentCommissionRateField& comRate){
		assert(!instrumentID.empty());
		const std::string& prodID = CommonUtils::CommonUtils::InstrumentIDToProductID(instrumentID);
		bool prod_existed = (m_InfoDict.find(prodID) != m_InfoDict.end());
		m_InfoDict[prodID].ComRateField = comRate;
		return prod_existed;
	}

	double InformationMgr::GetMargin(const std::string& instrumentID, int trade_volume, double trade_price, TThostFtdcDirectionType direction){
		const double invalid_init_ratio = 100;
		double margin_ratio_by_volume = invalid_init_ratio;
		double margin_ratio_by_money = invalid_init_ratio;

		if (direction == THOST_FTDC_D_Buy){
			margin_ratio_by_money = InstrumentManager.Get(instrumentID).MgrRateField.LongMarginRatioByMoney;
			margin_ratio_by_volume = InstrumentManager.Get(instrumentID).MgrRateField.LongMarginRatioByVolume;
		}
		else if (direction == THOST_FTDC_D_Sell){
			margin_ratio_by_money = InstrumentManager.Get(instrumentID).MgrRateField.ShortMarginRatioByMoney;
			margin_ratio_by_volume = InstrumentManager.Get(instrumentID).MgrRateField.ShortMarginRatioByVolume;
		}
		else
			assert(false);

		if (margin_ratio_by_volume < std::numeric_limits<double>::min() /*margin_ratio_by_volume = 0.0*/){
			double delta_amount = trade_price * trade_volume * InstrumentManager.Get(instrumentID).InstruField.VolumeMultiple;
			return delta_amount * margin_ratio_by_money;
		}else{
			return margin_ratio_by_volume * trade_volume;
		} 
	}

	double InformationMgr::GetCommission(const std::string& instrumentID, int trade_volume, double trade_price, TThostFtdcOffsetFlagType flag){
		const double invalid_init_ratio = 100;
		double commission_ratio_by_volume = invalid_init_ratio;
		double commission_ratio_by_money = invalid_init_ratio;
		if (THOST_FTDC_OF_Open == flag){
			commission_ratio_by_money = InstrumentManager.Get(instrumentID).ComRateField.OpenRatioByMoney;
			commission_ratio_by_volume = InstrumentManager.Get(instrumentID).ComRateField.OpenRatioByVolume;
		}
		else if (THOST_FTDC_OF_Close == flag ||  THOST_FTDC_OF_CloseYesterday == flag){
			commission_ratio_by_money = InstrumentManager.Get(instrumentID).ComRateField.CloseRatioByMoney;
			commission_ratio_by_volume = InstrumentManager.Get(instrumentID).ComRateField.CloseRatioByVolume;
		}
		else if (THOST_FTDC_OF_CloseToday == flag){
			commission_ratio_by_money = InstrumentManager.Get(instrumentID).ComRateField.CloseTodayRatioByMoney;
			commission_ratio_by_volume = InstrumentManager.Get(instrumentID).ComRateField.CloseTodayRatioByVolume;
		}
		else {
			assert(false);
		}

		if (commission_ratio_by_volume < std::numeric_limits<double>::min() /*commission_ratio_by_volume = 0.0*/){
			double delta_amount = trade_volume * trade_price * InstrumentManager.Get(instrumentID).InstruField.VolumeMultiple;
			return commission_ratio_by_money * delta_amount;
		}
		else
			return commission_ratio_by_volume * trade_volume;
	}

	std::string InformationMgr::AllInstruments() const{
		std::string ret = "";
		for (auto item : m_InfoVec){
			ret += item;
			ret += ", ";
		}
		return ret.substr(0, ret.size() - 1);
	}

	void InformationMgr::serialize() const {
		Json::Value root;

		for (auto instru : m_InfoDict){
			Json::Value instru_info;
			Json::Value instru_basic_info, margin_info, commission_info;
			
			instru_basic_info[kInstrumentID] = instru.second.InstruField.InstrumentID;
			instru_basic_info[kExchangeID] = instru.second.InstruField.ExchangeID;
			instru_basic_info[kDeliveryYear] = instru.second.InstruField.DeliveryYear;
			instru_basic_info[kDeliveryMonth] = instru.second.InstruField.DeliveryMonth;
			instru_basic_info[kVolumeMultiple] = instru.second.InstruField.VolumeMultiple;
			instru_basic_info[kOpenDate] = instru.second.InstruField.OpenDate;
			instru_basic_info[kExpireDate] = instru.second.InstruField.ExpireDate;
			instru_basic_info[kIsTrading] = instru.second.InstruField.IsTrading;
			instru_basic_info[kPositionType] = instru.second.InstruField.PositionType;

			margin_info[kBrokerID] = instru.second.MgrRateField.BrokerID;
			margin_info[kInvestorID] = instru.second.MgrRateField.InvestorID;
			margin_info[kHedgeFlag] = instru.second.MgrRateField.HedgeFlag;
			margin_info[kLongMarginRatioByMoney] = instru.second.MgrRateField.LongMarginRatioByMoney;
			margin_info[kLongMarginRatioByVolume] = instru.second.MgrRateField.LongMarginRatioByVolume;
			margin_info[kShortMarginRatioByMoney] = instru.second.MgrRateField.ShortMarginRatioByMoney;
			margin_info[kShortMarginRatioByVolume] = instru.second.MgrRateField.ShortMarginRatioByVolume;

			commission_info[kOpenRatioByMoney] = instru.second.ComRateField.OpenRatioByMoney;
			commission_info[kOpenRatioByVolume] = instru.second.ComRateField.OpenRatioByVolume;
			commission_info[kCloseRatioByMoney] = instru.second.ComRateField.CloseRatioByMoney;
			commission_info[kCloseRatioByVolume] = instru.second.ComRateField.CloseRatioByVolume;
			commission_info[kCloseTodayRatioByMoney] = instru.second.ComRateField.CloseTodayRatioByMoney;
			commission_info[kCloseTodayRatioByVolume] = instru.second.ComRateField.CloseTodayRatioByVolume;

			instru_info[kBasicInfo] = instru_basic_info;
			instru_info[kMarginInfo] = margin_info;
			instru_info[kCommissionInfo] = commission_info;

			root[instru.first] = instru_info;
		}
		std::ofstream out_json_file(FactorSettingFilePath);
		Json::StyledStreamWriter writer;
		writer.write(out_json_file, root);
		out_json_file.close();
	}
	
	void InformationMgr::unserialize(){
		Json::Reader reader;

		Json::Value root;

		std::ifstream in_json_file(FactorSettingFilePath);


		if (reader.parse(in_json_file, root, false) == false || root.type()==Json::nullValue){
			m_isSetup = false;
			return;	
		}

		m_InfoDict.clear();

		for (auto key : root.getMemberNames()){
			Information instu_info;
			STRCPY(instu_info.InstruField.ProductID, key.c_str());
			STRCPY(instu_info.InstruField.InstrumentID, root[key][kBasicInfo][kInstrumentID].asString().c_str());
			STRCPY(instu_info.InstruField.ExchangeID, root[key][kBasicInfo][kExchangeID].asString().c_str());
			instu_info.InstruField.DeliveryYear = root[key][kBasicInfo][kDeliveryYear].asInt();
			instu_info.InstruField.DeliveryMonth = root[key][kBasicInfo][kDeliveryMonth].asInt();
			instu_info.InstruField.VolumeMultiple = root[key][kBasicInfo][kVolumeMultiple].asInt();
			STRCPY(instu_info.InstruField.OpenDate, root[key][kBasicInfo][kOpenDate].asString().c_str());
			STRCPY(instu_info.InstruField.ExpireDate, root[key][kBasicInfo][kExpireDate].asString().c_str());
			instu_info.InstruField.IsTrading = root[key][kBasicInfo][kIsTrading].asInt();
			instu_info.InstruField.PositionType = root[key][kBasicInfo][kPositionType].asUInt();

			STRCPY(instu_info.MgrRateField.BrokerID, root[key][kMarginInfo][kBrokerID].asString().c_str());
			STRCPY(instu_info.MgrRateField.InvestorID, root[key][kMarginInfo][kInvestorID].asString().c_str());
			instu_info.MgrRateField.HedgeFlag = root[key][kMarginInfo][kHedgeFlag].asUInt();
			instu_info.MgrRateField.LongMarginRatioByMoney = root[key][kMarginInfo][kLongMarginRatioByMoney].asDouble();
			instu_info.MgrRateField.LongMarginRatioByVolume = root[key][kMarginInfo][kLongMarginRatioByVolume].asDouble();
			instu_info.MgrRateField.ShortMarginRatioByMoney = root[key][kMarginInfo][kShortMarginRatioByMoney].asDouble();
			instu_info.MgrRateField.ShortMarginRatioByVolume = root[key][kMarginInfo][kShortMarginRatioByVolume].asDouble();

			instu_info.ComRateField.OpenRatioByMoney = root[key][kCommissionInfo][kOpenRatioByMoney].asDouble();
			instu_info.ComRateField.OpenRatioByVolume = root[key][kCommissionInfo][kOpenRatioByVolume].asDouble();
			instu_info.ComRateField.CloseRatioByMoney = root[key][kCommissionInfo][kCloseRatioByMoney].asDouble();
			instu_info.ComRateField.CloseRatioByVolume = root[key][kCommissionInfo][kCloseRatioByVolume].asDouble();
			instu_info.ComRateField.CloseTodayRatioByMoney = root[key][kCommissionInfo][kCloseTodayRatioByMoney].asDouble();
			instu_info.ComRateField.CloseTodayRatioByVolume = root[key][kCommissionInfo][kCloseTodayRatioByVolume].asDouble();

			Add(key, instu_info);
		}

		m_isSetup = true;
		in_json_file.close();
	}

	TThostFtdcExchangeIDTypeType InformationMgr::GetExchangeID(const std::string& instrumentID) const{
		const std::string& prodID = CommonUtils::InstrumentIDToProductID(instrumentID);
		auto iter = m_InfoDict.find(prodID);
		if (iter != m_InfoDict.end()){
			if (strcmp("SHFE", iter->second.InstruField.ExchangeID) == 0){
				return THOST_FTDC_EIDT_SHFE;
			}
			else if (strcmp("CZCE", iter->second.InstruField.ExchangeID) == 0){
				return THOST_FTDC_EIDT_CZCE;
			}
			else if (strcmp("DCE", iter->second.InstruField.ExchangeID) == 0){
				return THOST_FTDC_EIDT_DCE;
			}
			else if (strcmp("CFFEX", iter->second.InstruField.ExchangeID) == 0){
				return THOST_FTDC_EIDT_CFFEX;
			}
			else if (strcmp("INE", iter->second.InstruField.ExchangeID) == 0)
			{
				return THOST_FTDC_EIDT_INE;
			}
			else
				return THOST_FTDC_EIDT_WRONG;
		}
		else{
			return THOST_FTDC_EIDT_WRONG;
		}
	}
}

Instrument::InformationMgr& InstrumentManager = Instrument::GetManager();
