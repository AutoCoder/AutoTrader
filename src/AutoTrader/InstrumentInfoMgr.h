#ifndef BASE_ACCOUNT_H
#define BASE_ACCOUNT_H

#include "ThostFtdcUserApiDataType.h"
#include <map>

namespace Instrument{

	struct Information{
	public:
		Information(){};
		Information(const TThostFtdcVolumeMultipleType& mul) :multiple(mul) {};

		TThostFtdcVolumeMultipleType multiple;

		//todo: add property
		// is_ZhuLi
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
	