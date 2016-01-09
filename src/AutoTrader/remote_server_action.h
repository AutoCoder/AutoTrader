#ifndef SERVER_ACTION_H
#define SERVER_ACTION_H
#include <string>
#include "socket_session.h"

namespace Transmission{

	class RemoteServerAction
	{
	public:
		RemoteServerAction(const std::shared_ptr<socket_session>& session,const std::string& data);
		~RemoteServerAction();

		void Invoke();

	private:
		std::string                       original_data_;
		std::shared_ptr<socket_session>   session_;
	};
}
#endif