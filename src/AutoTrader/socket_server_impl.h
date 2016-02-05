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
				// todo: 1)close socket
				// 2)remove session from server 
				//## [2016-2-5] for now, the expired session have no way to removed from session manager. but it doesn't affect a lot about feature
				socket_.close();
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
			std::shared_ptr<tcp::acceptor> acceptor_;
			tcp::socket socket_;
		};
	}
}
#endif