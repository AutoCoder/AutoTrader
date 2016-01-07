#include "stdafx.h"

#include <memory>
#include "socket_server_impl.h"
#include "remote_user_action.h"

namespace Transmission{

	namespace Detail{

		session::session(tcp::socket socket)
			: socket_(std::move(socket))
			, user_action_(this)
		{
		}

		void session::start()
		{
			do_read();
		}

		void session::do_read()
		{
			auto self(shared_from_this());
			socket_.async_read_some(boost::asio::buffer(buffer_, max_length),
				[this, self](boost::system::error_code ec, std::size_t bytes_transferred)
			{
				if (!ec)
				{

					RemoteUserAction::Parse_Result result = user_action_.Parse(
						buffer_, buffer_ + bytes_transferred);

					if (result == RemoteUserAction::indeterminate){
						//do nothing
					}
					else{
						std::string immediatelyReply;
						if (result == RemoteUserAction::Good)
						{
							immediatelyReply = "Valid Action is received, responsing...";
							//todo : send signal(UserAction Object) to strategy engine
						}
						else if (result == RemoteUserAction::Bad){
							immediatelyReply = "Invalid Action is received, ignored...";	
						}
						do_write(immediatelyReply.c_str(), immediatelyReply.length());
						user_action_.Reset();//start to receive new Action Request
					}

					do_read();
				}
				else{
					assert(false);
					on_socket_error();
				}
			});
		}


		void session::do_write(const char* reply, size_t size){
			auto self(shared_from_this());
			boost::asio::async_write(socket_, boost::asio::buffer(reply, size),
				[this, self](boost::system::error_code ec, std::size_t /*length*/)
			{
				if (ec)
				{
					//fail to reply
					//todo : add log
					on_socket_error();
				}
			});
		}

		socket_server::socket_server(short port)
			: io_service_()
			, acceptor_(io_service_, tcp::endpoint(tcp::v4(), port))
			, socket_(io_service_)
		{
			do_accept();
		}

		void socket_server::run(){
			io_service_.run();
		}

		void socket_server::do_accept()
		{
			acceptor_.async_accept(socket_,
				[this](boost::system::error_code ec)
			{
				if (!ec)
				{
					std::make_shared<session>(std::move(socket_))->start();
				}

				do_accept();
			});
		}

	}
}