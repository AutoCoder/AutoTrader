#ifndef TICK_WRAPPER_H
#define TICK_WRAPPER_H

#include "stdafx.h"
#include "ThostFtdcUserApiStruct.h"
#include <vector>
#include <string>
#include <memory>
#include "TechVec.h"

typedef std::vector<std::string> CThostFtdcDepthMDFieldDBStruct;
class DBWrapper;
class STRATEGY_API TickWrapper
{
public:
	TickWrapper();
	TickWrapper(CThostFtdcDepthMarketDataField* p);
	TickWrapper(const TickWrapper& obj);
	TickWrapper& operator = (const TickWrapper& obj);
	TickWrapper(TickWrapper && obj);

	~TickWrapper();

	bool IsEmpty() const { return m_isEmpty; }

	void serializeToDB(DBWrapper& db) const;

	// return value:(unit: half second = 500ms, time to 19700101 00:00:00 + UTC8)
	long long toTimeStamp() const;

	long long FirstSecondsTimeStamp() const;

	double TurnOver() const{
		return m_MdData.Turnover;
	}

	long long Volume() const{
		return m_MdData.Volume;
	}

	double BidPrice1() const{
		return m_MdData.BidPrice1;
	}

	double BidVolume1() const{
		return m_MdData.BidVolume1;
	}

	double AskPrice1() const {
		return m_MdData.AskPrice1;
	}

	double AskVolume1() const {
		return m_MdData.AskVolume1;
	}

	double LowestPrice() const{
		return m_MdData.LowestPrice;
	}

	double HighestPrice() const {
		return m_MdData.HighestPrice;
	}

	double OpenPrice() const{
		return m_MdData.OpenPrice;
	}

	double ClosePrice() const {
		return m_MdData.ClosePrice;
	}

	double PreSettlementPrice() const {
		return m_MdData.PreSettlementPrice;
	}

	double SettlementPrice() const {
		return m_MdData.SettlementPrice;
	}

	double PreClosePrice() const {
		return m_MdData.PreClosePrice;
	}

	double UpperLimitPrice() const {
		return m_MdData.UpperLimitPrice;
	}

	double LowerLimitPrice() const{
		return m_MdData.LowerLimitPrice;
	}

	inline double LastPrice() const{
		return m_MdData.LastPrice;
	}

	inline std::string InstrumentId() const{
		return std::string(m_MdData.InstrumentID);
	}

	StrategyTech* GetTechVec() const{
		return m_techvec;
	}

	void SetTechVec(StrategyTech* p){
		m_techvec = p;
	}

	long long UUID() const {
		return m_uuid;
	}

	std::string UpdateTime() const {
		return m_MdData.UpdateTime;
	}

	std::string Time() const {
		std::string ret = m_MdData.TradingDay;
		ret += " ";
		ret += m_MdData.UpdateTime;
		return ret;
	}

	static TickWrapper RecoverFromDB(const CThostFtdcDepthMDFieldDBStruct& vec);

private:
	CThostFtdcDepthMarketDataField m_MdData;
	long long m_uuid;
	bool recoveryData;
	bool m_isEmpty;
	//std::shared_ptr<TechVec> m_techvec;
public:
	StrategyTech* m_techvec;
};

#endif