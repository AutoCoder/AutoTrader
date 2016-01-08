#ifndef SOCKET_SERVER_IMPL_H
#define SOCKET_SERVER_IMPL_H

#include <memory>
#include <boost/utility.hpp>
#include <boost/asio.hpp>
#include "remote_user_action.h"

namespace Transmission{

	namespace Detail{

		using boost::asio::ip::tcp;

		class session
			: public std::enable_shared_from_this<session>
		{
		public:
			session(tcp::socket socket);
			void start();
			void do_write(const char* bytes, size_t size);

		private:
			void do_read();
			void on_socket_error() { 
				// todo: close socket
				// remove session from server
			};

		private:
			tcp::socket socket_;
			enum { max_length = 1024 };
			char buffer_[max_length];
			RemoteUserAction user_action_;
		};


		class server : public boost::noncopyable
		{
		public:
			explicit server(short port);
			void run();

		private:
			void do_accept();

		private:
			boost::asio::io_service io_service_;
			tcp::acceptor acceptor_;
			tcp::socket socket_;
		};
	}
}
#endif