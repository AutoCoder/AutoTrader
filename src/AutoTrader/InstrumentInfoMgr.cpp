#include "stdafx.h"
#include "InstrumentInfoMgr.h"
#include <assert.h>

#define instrument_1 "rb1601"
#define instrument_2 "rb1602"

namespace Instrument{

	InformationMgr::InformationMgr()
	{
		m_InfoDict.clear();
		m_InfoDict[instrument_1] = Information(10);
		m_InfoDict[instrument_2] = Information(10);
	}


	InformationMgr::~InformationMgr()
	{
	}


	TThostFtdcVolumeMultipleType InformationMgr::GetVolumeMultiple(const std::string& instr) const{
		if (m_InfoDict.find(instr) != m_InfoDict.end())
			// operator[] may change the inside data, it don't have const version, so here it's should be "at()"
			return m_InfoDict.at(instr).multiple;
		else{
			//"should be added into this InstrumentInfoMgr"
			assert(false);
			return -10000;
		}
	}

	InformationMgr& GetManager(){
		static InformationMgr mgr;
		return mgr;
	}

}