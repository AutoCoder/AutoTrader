#include "AccontMetaMgr.h"

namespace Account{

	void Manager::AddAccontMeta(const Meta& acm){
		m_metas[acm.Id()] = acm;
	}

	void Manager::RemoveAccontMeta(const std::string& id){
		m_metas.erase(id);
	}

	bool Manager::IsAccountExisted(const std::string& aId){
		return m_metas.find(aId) != m_metas.end();
	}

	bool Manager::CheckPassword(const std::string& aId, const std::string& pw){
		if (IsAccountExisted(aId)){
			return m_metas.find(aId)->second.m_Password == pw;
		}
		return false;
	}

	const Meta& Manager::GetMeta(const std::string& aId){
		return m_metas[aId];
	}
}