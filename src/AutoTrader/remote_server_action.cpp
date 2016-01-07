#include "stdafx.h"
#include "remote_server_action.h"
#include "socket_server_impl.h"
namespace Transmission{
	RemoteServerAction::RemoteServerAction(Transmission::socket_session_ptr session)
		:session_(session)
	{
	}


	RemoteServerAction::~RemoteServerAction()
	{
	}


	void RemoteServerAction::Invoke(){
		if (session_)
			session_->do_write(reinterpret_cast<char*>(original_data_.data()), original_data_.size());
	}

}