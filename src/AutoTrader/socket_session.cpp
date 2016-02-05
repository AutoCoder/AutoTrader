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
	
	bool socket_session::operator == (const socket_session& rhs) const{
		if (rhs.session_impl_.expired() || session_impl_.expired())
			return false;
		auto rhs_sp = rhs.session_impl_.lock();
		auto sp = session_impl_.lock();
		return rhs_sp.get() == sp.get();
	}
}