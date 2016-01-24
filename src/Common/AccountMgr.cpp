#include "AccountMgr.h"
#include <set>

namespace Account{

	std::vector<std::string> Manager::Instruments() const{
		std::set<std::string> instrus;
		for (auto item : m_metas){
			for (auto sub : item.second.m_Instruments){
				instrus.insert(sub);
			}
		}
		std::vector<std::string> ret;
		ret.assign(instrus.begin(), instrus.end());
		return ret;
	}

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