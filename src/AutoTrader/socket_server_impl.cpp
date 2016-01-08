#include "stdafx.h"

#include <memory>
#include "socket_server_impl.h"
#include "remote_user_action.h"
#include "fifo_action_queue.h"
#include "socket_session.h" // notice recursive include 

namespace Transmission{

	namespace Detail{

		session::session(tcp::socket socket)
			: socket_(std::move(socket))
			, user_action_(std::make_shared<socket_session>(this))
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

					if (result == RemoteUserAction::Parse_Result::indeterminate){
						//do nothing
					}
					else{
						std::string immediatelyReply;
						if (result == RemoteUserAction::Parse_Result::Good)
						{
							immediatelyReply = "Valid Action is received, responsing...";
							//send (UserAction Object) to fifo queue for handling...
							GetFIFOActionQueue().Push_back(user_action_);
						}
						else if (result == RemoteUserAction::Parse_Result::Bad){
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

		server::server(short port)
			: io_service_()
			, acceptor_(io_service_, tcp::endpoint(tcp::v4(), port))
			, socket_(io_service_)
		{
			do_accept();
		}

		void server::run(){
			io_service_.run();
		}

		void server::do_accept()
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