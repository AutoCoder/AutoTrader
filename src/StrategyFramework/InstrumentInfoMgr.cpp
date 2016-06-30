#include "stdafx.h"
#include "InstrumentInfoMgr.h"

#include "json/json.h"
#include <assert.h>

namespace Instrument{

	static const char* FactorSettingFilePath = "InstrumentInfo.json";

	//Three Group
	static const char* kBasicInfo = "BasicInfo";
	static const char* kMarginInfo = "MarginInfo";
	static const char* kCommissionInfo = "CommissionInfo";

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
	}


	InformationMgr::~InformationMgr()
	{
	}

	const Information& InformationMgr::Get(const std::string& instrumentID) const{
		assert (m_InfoDict.find(instrumentID) != m_InfoDict.end());
		return m_InfoDict.at(instrumentID);
	}

	void InformationMgr::Add(const std::string& instrumentID, const Information& info){
		assert(!m_isSetup);
		if (m_InfoDict.find(instrumentID) == m_InfoDict.end()){
			m_InfoDict.insert(std::make_pair(instrumentID, info));
		}
	}

	bool InformationMgr::SetMarginRate(const std::string& instrumentID, const CThostFtdcInstrumentMarginRateField& mgrRate){
		assert(!m_isSetup);
		if (m_InfoDict.find(instrumentID) != m_InfoDict.end()){
			m_InfoDict[instrumentID].MgrRateField = mgrRate;
			return true;
		}
		return false;
	}

	bool InformationMgr::SetCommissionRate(const std::string& instrumentID, const CThostFtdcInstrumentCommissionRateField& comRate){
		assert(!m_isSetup);
		if (m_InfoDict.find(instrumentID) != m_InfoDict.end()){
			m_InfoDict[instrumentID].ComRateField = comRate;
			return true;
		}
		return false;
	}

	std::string InformationMgr::AllInstruments() const{
		std::string ret = "";
		for (auto item : m_InfoDict){
			ret += item.first;
			ret += ", ";
		}
		return ret.substr(0, ret.size() - 1);
	}

	void InformationMgr::serialize() const {
		Json::Value root;

		for (auto instru : m_InfoDict){
			Json::Value instru_info;
			Json::Value instru_basic_info, margin_info, commission_info;
			instru.second.InstruField.ExchangeID

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
		std::ofstream jsonfile(FactorSettingFilePath);
		Json::StreamWriter writer;
		writer.write(root, &jsonfile);
	}
	
	void InformationMgr::unserialize(){
		Json::Reader reader;

		Json::Value root;

		std::ifstream jsonfile(FactorSettingFilePath);


		if (reader.parse(jsonfile, root, false) == false || root.type()==Json::nullValue){
			m_isSetup = false;
			return;	
		}

		m_InfoDict.clear();

		for (auto key : root.Members){
			Information instu_info;
			strcpy(instu_info.InstruField.InstrumentID, key.c_str());
			strcpy(instu_info.InstruField.ExchangeID, root[key][kExchangeID].c_str());
			strcpy(instu_info.InstruField.DeliveryYear, root[key][kDeliveryYear].c_str());
			strcpy(instu_info.InstruField.DeliveryMonth, root[key][kDeliveryMonth].c_str());
			instu_info.InstruField.VolumeMultiple = root[key][kVolumeMultiple];
			strcpy(instu_info.InstruField.OpenDate, root[key][kOpenDate].c_str());
			strcpy(instu_info.InstruField.ExpireDate, root[key][kExpireDate].c_str());
			instu_info.InstruField.IsTrading = root[key][kIsTrading];
			instu_info.InstruField.PositionType = root[key][kPositionType];

			strcpy(instu_info.MgrRateField.BrokerID, root[key][kBrokerID].c_str());
			strcpy(instu_info.MgrRateField.InvestorID, root[key][kInvestorID].c_str());
			instu_info.MgrRateField.HedgeFlag = root[key][kHedgeFlag];
			instu_info.MgrRateField.LongMarginRatioByMoney = root[key][kLongMarginRatioByMoney];
			instu_info.MgrRateField.LongMarginRatioByVolume = root[key][kLongMarginRatioByVolume];
			instu_info.MgrRateField.ShortMarginRatioByMoney = root[key][kShortMarginRatioByMoney];
			instu_info.MgrRateField.ShortMarginRatioByVolume = root[key][kShortMarginRatioByVolume];

			instu_info.ComRateField.OpenRatioByMoney = root[key][kOpenRatioByMoney];
			instu_info.ComRateField.OpenRatioByVolume = root[key][kOpenRatioByVolume];
			instu_info.ComRateField.CloseRatioByMoney = root[key][kCloseRatioByMoney];
			instu_info.ComRateField.CloseRatioByVolume = root[key][kCloseRatioByVolume];
			instu_info.ComRateField.CloseTodayRatioByMoney = root[key][kCloseTodayRatioByMoney];
			instu_info.ComRateField.CloseTodayRatioByVolume = root[key][kCloseTodayRatioByVolume];

			Add(key, instu_info);
		}
		m_isSetup = true;
	}
}

Instrument::InformationMgr& InstrumentManager = Instrument::GetManager();