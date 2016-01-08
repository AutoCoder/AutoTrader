#include "stdafx.h"
#include "socket_session.h"
#include "socket_server_impl.h"


namespace Transmission{

	socket_session::socket_session(Detail::session* innerPtr):
		session_impl_(std::shared_ptr<Detail::session>(innerPtr)) //Comment: weak_ptr can't be constructed with a native ptr, constructed by a shared_ptr is ok
	{
	}

	bool socket_session::do_write(const char* bytes, size_t size)
	{
		auto sharedPtr = session_impl_.lock();
		if (sharedPtr){
			sharedPtr->do_write(bytes, size);
			return true;
		}
		else
			return false;
	}

}