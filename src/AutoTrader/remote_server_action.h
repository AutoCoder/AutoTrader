#ifndef SERVER_ACTION_H
#define SERVER_ACTION_H
#include <vector>
#include "socket_server.h"

namespace Transmission{

	class RemoteServerAction
	{
	public:
		RemoteServerAction(Transmission::socket_session_ptr session);
		~RemoteServerAction();

		const std::vector<char>& Bytes() { return original_data_; }
		void Invoke();

	private:
		std::vector<char>                 original_data_;
		Transmission::socket_session_ptr  session_;
	};
}
#endif