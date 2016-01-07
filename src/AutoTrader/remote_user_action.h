#ifndef USER_ACTION_H
#define USER_ACTION_H

#include <vector>
#include "socket_server.h"

namespace Transmission{

	class RemoteUserAction
	{
	public:
		RemoteUserAction(){};
		explicit RemoteUserAction(Transmission::socket_session_ptr session);
		~RemoteUserAction();

		enum Parse_Result {
			Good,
			Bad,
			indeterminate
		};

		Parse_Result Parse(char* begin, char* end);
		void Invoke();
		void Reset();
		const std::vector<char>& Bytes() { return original_data_; }

	private:
		size_t                            length_;
		std::vector<char>                 original_data_;
		Transmission::socket_session_ptr  session_;

	};
}
#endif 