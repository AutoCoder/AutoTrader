#include "stdafx.h"
#include "socket_session.h"
#include "socket_server_impl.h"


namespace Transmission{

	socket_session::socket_session(const std::shared_ptr<Detail::session>& impl_ptr)
		:session_impl_(impl_ptr)
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