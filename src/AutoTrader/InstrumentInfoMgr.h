#ifndef INSTRUMENT_MGR_H 
#define INSTRUMENT_MGR_H

#include "ThostFtdcUserApiDataType.h"
#include <map>

namespace Instrument{

	struct Information{
	public:
		Information(){};
		Information(const TThostFtdcVolumeMultipleType& mul) :multiple(mul) {};

		TThostFtdcVolumeMultipleType multiple;

	};

	class InformationMgr
	{
	public:
		InformationMgr();
		~InformationMgr();
		TThostFtdcVolumeMultipleType GetVolumeMultiple(const std::string& instr) const;

	private:
		InformationMgr(const InformationMgr& mgr) = delete;
		InformationMgr& operator = (const InformationMgr& mgr) = delete;

	private:
		//the pair-first is instrument Name
		std::map< std::string, Information > m_InfoDict;
	};

	InformationMgr& GetManager();
};


#endif
	