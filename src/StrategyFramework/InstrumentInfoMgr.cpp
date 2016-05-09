#include "stdafx.h"
#include "InstrumentInfoMgr.h"
#include <assert.h>

namespace Instrument{

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
		if (m_InfoDict.find(instrumentID) == m_InfoDict.end()){
			m_InfoDict.insert(std::make_pair(instrumentID, info));
		}
	}

	bool InformationMgr::SetMarginRate(const std::string& instrumentID, const CThostFtdcInstrumentMarginRateField& mgrRate){
		if (m_InfoDict.find(instrumentID) != m_InfoDict.end()){
			m_InfoDict[instrumentID].MgrRateField = mgrRate;
			return true;
		}
		return false;
	}

	bool InformationMgr::SetCommissionRate(const std::string& instrumentID, const CThostFtdcInstrumentCommissionRateField& comRate){
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
}

Instrument::InformationMgr& InstrumentManager = Instrument::GetManager();