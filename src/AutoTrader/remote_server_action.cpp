#include "stdafx.h"
#include "remote_server_action.h"

namespace Transmission{
	RemoteServerAction::RemoteServerAction(const std::shared_ptr<socket_session>& session, const std::string& data)
		: original_data_(data)
		, session_(session)
	{
	}


	RemoteServerAction::~RemoteServerAction()
	{
	}


	void RemoteServerAction::Invoke(){
		session_->do_write(original_data_.c_str(), original_data_.size());
	}

}