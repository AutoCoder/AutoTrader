#include "stdafx.h"
#include "remote_server_action.h"

namespace Transmission{
	RemoteServerAction::RemoteServerAction(const Transmission::socket_session& session)
		:session_(session)
	{
	}


	RemoteServerAction::~RemoteServerAction()
	{
	}


	void RemoteServerAction::Invoke(){
		session_.do_write(original_data_.c_str(), original_data_.size());
	}

}