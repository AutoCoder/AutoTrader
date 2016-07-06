#ifndef INSTRUMENT_MGR_H 
#define INSTRUMENT_MGR_H

#include "ThostFtdcUserApiDataType.h"
#include "ThostFtdcUserApiStruct.h"
#include <map>
#include "stdafx.h"
#include "InstrumentInfoMgr.h"

namespace Instrument{


	//期货期权手续费字段合并
	struct STRATEGY_API InstFee
	{
		TThostFtdcRatioType	OpenRatioByMoney;
		TThostFtdcRatioType	OpenRatioByVolume;
		TThostFtdcRatioType	CloseRatioByMoney;
		TThostFtdcRatioType	CloseRatioByVolume;
		TThostFtdcRatioType	CloseTodayRatioByMoney;
		TThostFtdcRatioType	CloseTodayRatioByVolume;
		TThostFtdcRatioType StrikeRatioByMoney;
		TThostFtdcRatioType StrikeRatioByVolume;
#ifdef _REAL_CTP_
		TThostFtdcRatioType	OrderCommByVolume;
		TThostFtdcRatioType	OrderActionCommByVolume;
#endif
	};

	struct STRATEGY_API OpMgrate //期权保证金率
	{
		TThostFtdcMoneyType	FixedMargin;
		TThostFtdcMoneyType	MiniMargin;
		TThostFtdcMoneyType	Royalty;
		TThostFtdcMoneyType	ExchFixedMargin;
		TThostFtdcMoneyType	ExchMiniMargin;
	};

	struct STRATEGY_API Information
	{
		Information(){};
		Information(const CThostFtdcInstrumentField& ins_field)
		{
			memcpy(&InstruField, &ins_field, sizeof(CThostFtdcInstrumentField));
			memset(&MgrRateField, 0, sizeof(CThostFtdcInstrumentMarginRateField));
			memset(&ComRateField, 0, sizeof(CThostFtdcInstrumentCommissionRateField));
		};

		Information& operator = (const Information& info){
			memcpy(&InstruField, &info.InstruField, sizeof(CThostFtdcInstrumentField));
			memcpy(&MgrRateField, &info.MgrRateField, sizeof(CThostFtdcInstrumentMarginRateField));
			memcpy(&ComRateField, &info.ComRateField, sizeof(CThostFtdcInstrumentCommissionRateField));
		}

		CThostFtdcInstrumentField					InstruField;
		CThostFtdcInstrumentMarginRateField			MgrRateField;
		CThostFtdcInstrumentCommissionRateField		ComRateField;
	};

	class STRATEGY_API InformationMgr
	{
	public:
		typedef std::map< std::string, Information > InstrumentDict;
		typedef InstrumentDict::iterator             InstrumentDictIterator;
		typedef InstrumentDict::const_iterator       InstrumentDictConstIterator;
		InformationMgr();
		~InformationMgr();

		InformationMgr(const InformationMgr& mgr) = delete;
		InformationMgr& operator = (const InformationMgr& mgr) = delete;

	public:
		const Information& Get(const std::string& instrumentID) const;
		void Add(const std::string& instrumentID, const Information& info);
		bool SetMarginRate(const std::string& instrumentID, const CThostFtdcInstrumentMarginRateField& mgrRate);
		bool SetCommissionRate(const std::string& instrumentID, const CThostFtdcInstrumentCommissionRateField& comRate);
		std::string AllInstruments() const;
		InstrumentDictIterator begin() { return m_InfoDict.begin(); } ;
		InstrumentDictConstIterator end() const { return m_InfoDict.end(); };
		void serialize() const;
	private:
		void unserialize();
	private:
		//the pair-first is instrument Name
		std::map< std::string, Information > m_InfoDict;
		std::vector< std::string >           m_InfoVec;
		bool m_isSetup;
	};

	//InformationMgr& GetManager();
	
};

extern STRATEGY_API Instrument::InformationMgr& InstrumentManager;

#endif
	