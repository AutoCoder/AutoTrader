#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H
#include <memory>

//This wrapper is to avoid include <boost/asio.hpp> to outside.

namespace Transmission{

	namespace Detail{
		class server;
		class session;
	}

	class socket_server
	{
	public:
		socket_server(short port);
		void run();/// Run the server's io_service loop.
		void stop(); // stop io_service

	private:
		std::shared_ptr<Detail::server> server_impl_;
	};
}

#endif