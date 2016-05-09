#ifndef INSTRUMENT_MGR_H 
#define INSTRUMENT_MGR_H

#include "ThostFtdcUserApiDataType.h"
#include "ThostFtdcUserApiStruct.h"
#include <map>

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
			:InstruField(ins_field)
		{
			
		};

		CThostFtdcInstrumentField					InstruField;
		CThostFtdcInstrumentMarginRateField			MgrRateField;
		CThostFtdcInstrumentCommissionRateField		ComRateField;
		//期货保证金率
		TThostFtdcRatioType	LongMgrByMoney;
		TThostFtdcMoneyType	LongMgrByVolume;
		TThostFtdcRatioType	ShortMgrByMoney;
		TThostFtdcMoneyType	ShortMgrByVolume;
		OpMgrate	omgr;			//期权保证金率
		InstFee		fee;			//合并的手续费率
		TThostFtdcLargeVolumeType	OpenInterest;
		TThostFtdcMoneyType	Turnover;
		TThostFtdcPriceType	UpLmtPx;
		TThostFtdcPriceType	LowLmtPx;
	};

	class STRATEGY_API InformationMgr
	{
	public:
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
	private:
		//the pair-first is instrument Name
		std::map< std::string, Information > m_InfoDict;
	};

	//InformationMgr& GetManager();
	
};

extern STRATEGY_API Instrument::InformationMgr& InstrumentManager;

#endif
	