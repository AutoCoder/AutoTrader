#include "AccontMetaMgr.h"

namespace Accout{

	void MetaMgr::AddAccontMeta(const Meta& acm){
		m_metas[acm.Id()] = acm;
	}

	void MetaMgr::RemoveAccontMeta(const std::string& id){
		m_metas.erase(id);
	}
}