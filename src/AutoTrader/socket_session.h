#ifndef SOCKET_SESSION_H
#define SOCKET_SESSION_H
#include <memory>

namespace Transmission{

	namespace Detail{
		class session;
	}

	class socket_session
	{
	public:
		explicit socket_session(Detail::session* innerPtr);
		bool do_write(const char* reply, size_t size);

	private:
		// don't own the inner session, it's should be clean when the session recursive reading stop.
		std::weak_ptr<Detail::session> session_impl_;
	};

}
#endif