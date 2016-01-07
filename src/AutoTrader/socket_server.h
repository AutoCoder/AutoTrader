#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H
#include <memory>

namespace Transmission{

	namespace Detail{
		class socket_server;
		class session;
	}

	class socket_server
	{
	public:
		socket_server(short port);
		void run();/// Run the server's io_service loop.

	private:
		std::shared_ptr<Detail::socket_server> server_impl_;
	};

	typedef Detail::session* socket_session_ptr;
}

#endif