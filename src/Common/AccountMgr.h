#ifndef ACCOUNT_META_MGR
#define ACCOUNT_META_MGR

#include <string>
#include <map>
#include <vector>
#include "Common.h"

namespace Account{

	struct COMMON_API Meta{
		Meta(){};

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

	class COMMON_API Manager
	{
	public:
		~Manager(){};

		static Manager& Instance(){
			static Manager _mgr; //Note: Meryers singleton, not thread-safe, postpone thread-safe enhancement for all singletons in solution.
			return _mgr;
		}

		std::vector<std::string> Instruments() const;

		void AddAccontMeta(const Meta& acm);

		void RemoveAccontMeta(const std::string& id);

		bool IsAccountExisted(const std::string& aId);

		bool CheckPassword(const std::string& aId, const std::string& pw);

		const Meta& GetMeta(const std::string& aId);

	private:
		Manager(){}
		Manager(const Manager&) = delete;
		Manager& operator=(const Manager &) = delete;
	private:
		std::map<std::string, Meta> m_metas;
	};



}
#endif