#include "stdafx.h"
#include "ThostFtdcDepthMDFieldWrapper.h"
#include "ThostFtdcUserApiStruct.h"

CThostFtdcDepthMDFieldWrapper::CThostFtdcDepthMDFieldWrapper(CThostFtdcDepthMarketDataField* p):
	m_innerPtr(p)
{
}


CThostFtdcDepthMDFieldWrapper::~CThostFtdcDepthMDFieldWrapper()
{
}

void CThostFtdcDepthMDFieldWrapper::serializeToDB() const {

}
