#pragma once

class CThostFtdcDepthMarketDataField;

class CThostFtdcDepthMDFieldWrapper
{
public:
	CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p);
	~CThostFtdcDepthMDFieldWrapper();

	void serializeToDB() const;

	// return value:(unit: half second = 500ms, time to 19700101 00:00:00 + UTC8)
	int toTimeStamp() const;

	inline double turnOver() const;

	inline int volume() const;

	inline void setK3(double input){
		m_k3m = input;
	}

	inline double K3(){
		return m_k3m;
	}

	inline void setK5(double input){
		m_k5m = input;
	}

	inline double K5(){
		return m_k5m;
	}

private:
	CThostFtdcDepthMarketDataField* m_innerPtr;
	double m_k5m;
	double m_k3m;
};


