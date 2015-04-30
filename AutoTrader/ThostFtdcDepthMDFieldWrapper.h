#ifndef THOST_FTDC_DEPTH_MD_FIELD_WRAPPER_H
#define THOST_FTDC_DEPTH_MD_FIELD_WRAPPER_H

#include "ThostFtdcUserApiStruct.h"
#include <vector>

typedef std::vector<std::string> CThostFtdcDepthMDFieldDBStruct;

class CThostFtdcDepthMDFieldWrapper
{
public:
	CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p);
	~CThostFtdcDepthMDFieldWrapper();

	void serializeToDB() const;

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

	static CThostFtdcDepthMDFieldWrapper RecoverFromDB(const CThostFtdcDepthMDFieldDBStruct& vec);

private:
	CThostFtdcDepthMarketDataField m_MdData;
	double m_k5m;
	double m_k3m;
	static bool firstlanuch;
};

#endif