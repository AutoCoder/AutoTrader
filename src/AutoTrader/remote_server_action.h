#ifndef SERVER_ACTION_H
#define SERVER_ACTION_H
#include <string>
#include "socket_session.h"

namespace Transmission{

	class RemoteServerAction
	{
	public:
		RemoteServerAction(const Transmission::socket_session& session);
		~RemoteServerAction();

		void Invoke();

	private:
		std::string                       original_data_;
		Transmission::socket_session      session_;
	};
}
#endif