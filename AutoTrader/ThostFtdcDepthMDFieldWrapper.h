#pragma once

class CThostFtdcDepthMarketDataField;

class CThostFtdcDepthMDFieldWrapper
{
public:
	CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p);
	~CThostFtdcDepthMDFieldWrapper();

	void serializeToDB() const;

private:
	static const char *DBhost;
	static const int Port;
	static const char *DBName;

private:
	CThostFtdcDepthMarketDataField* m_innerPtr;
};

const char *DBhost = "127.0.0.1";
const int Port = 3306;
const char *qiHuo = "qihuo";
