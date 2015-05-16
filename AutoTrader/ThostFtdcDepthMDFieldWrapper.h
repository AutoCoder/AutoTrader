#ifndef THOST_FTDC_DEPTH_MD_FIELD_WRAPPER_H
#define THOST_FTDC_DEPTH_MD_FIELD_WRAPPER_H

#include "ThostFtdcUserApiStruct.h"
#include <vector>

enum class TickType{
	Commom = 0x0,
	BuyPoint = 0x1,
	SellPoint = 0x2,
};

typedef std::vector<std::string> CThostFtdcDepthMDFieldDBStruct;
class DBWrapper;
class CThostFtdcDepthMDFieldWrapper
{
public:
	CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p);
	~CThostFtdcDepthMDFieldWrapper();

	void serializeToDB(DBWrapper& db) const;

	// return value:(unit: half second = 500ms, time to 19700101 00:00:00 + UTC8)
	long long toTimeStamp() const;

	double TurnOver() const{
		return m_MdData.Turnover;
	}

	long long Volume() const{
		return m_MdData.Volume;
	}

	inline void setK3(double input){
		m_k3m = input;
	}

	inline double K3() const {
		return m_k3m;
	}

	inline void setK5(double input){
		m_k5m = input;
	}

	inline double K5() const {
		return m_k5m;
	}

	inline double LastPrice() {
		return m_MdData.LastPrice;
	}

	inline std::string InstrumentId(){
		return std::string(m_MdData.InstrumentID);
	}

	inline void SetTickType(TickType type, int strategy_idx){
		m_ticktype[strategy_idx] = type;
	}

	static CThostFtdcDepthMDFieldWrapper RecoverFromDB(const CThostFtdcDepthMDFieldDBStruct& vec);

private:
	CThostFtdcDepthMarketDataField m_MdData;
	double m_k5m;
	double m_k3m;
	static bool firstlanuch;
	bool recoveryData;
	TickType m_ticktype[5];
};

#endif