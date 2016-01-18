#ifndef ACCOUNT_META_MGR
#define ACCOUNT_META_MGR

#include <string>
#include <map>
#include <vector>
#include "Common.h"

namespace Accout{

	struct COMMON_API Meta{

		Meta(const std::string& b, const std::string& un, const std::string& pw, const std::vector<std::string>& instus)
		: m_BrokerId(b)
		, m_UserId(un)
		, m_Password(pw)
		{
			m_Instruments.assign(instus.begin(), instus.end());
		}

		std::string Id() const{ return m_BrokerId + m_UserId; }

		std::string                 m_BrokerId;
		std::string					m_UserId;
		std::string					m_Password;
		std::vector<std::string>	m_Instruments;
	};

	class COMMON_API MetaMgr
	{
	public:
		MetaMgr();
		~MetaMgr(){};
		static MetaMgr* getInstance();

		void AddAccontMeta(const Meta& acm);

		void RemoveAccontMeta(const std::string& id);

		const std::vector<Meta>& AllAccontMetas();

	private:
		MetaMgr();
		MetaMgr(const MetaMgr&) = delete;
		MetaMgr& operator=(const MetaMgr &) = delete;
	private:
		std::map<std::string, Meta> m_metas;
	};

	MetaMgr& GetManager(){
		static MetaMgr _mgr; //Meryers singleton
		return _mgr;
	}

}
#endif