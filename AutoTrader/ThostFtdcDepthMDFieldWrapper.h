#pragma once

class CThostFtdcDepthMarketDataField;

class CThostFtdcDepthMDFieldWrapper
{
public:
	CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p);
	~CThostFtdcDepthMDFieldWrapper();

	void serializeToDB() const;



private:
	CThostFtdcDepthMarketDataField* m_innerPtr;
};


