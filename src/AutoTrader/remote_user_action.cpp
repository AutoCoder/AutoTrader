#include "stdafx.h"
#include <assert.h>
#include "remote_user_action.h"
#include "socket_server_impl.h"

namespace{
	bool is_digit(int c){
		return c >= '0' && c <= '9';
	}
}
namespace Transmission{

	RemoteUserAction::RemoteUserAction(Transmission::socket_session_ptr session)
		: length_(0)
		, session_(session)
	{
	}


	RemoteUserAction::~RemoteUserAction()
	{
	}


	RemoteUserAction::Parse_Result RemoteUserAction::Parse(char* begin, char* end){
		assert(begin);
		assert(end > begin);
		while (begin < end && original_data_.empty() && is_digit(*begin)){ // read header(msg length)
			length_ = 10 * length_ + (*begin - '0');
			begin++;
		}

		while (begin < end && original_data_.size() < length_){
			original_data_.push_back(*begin++);
		}

		if (begin == end)
		{
			if (original_data_.size() < length_){
				return Parse_Result::indeterminate;
			}
			else if (original_data_.size() == length_){
				//todo: parse json,verify json is valid or not.
				return Parse_Result::Good;
			}
			else{
				//it's impossible original_data_.size() > length_
				assert(false);
			}
		}
		else{
			return Parse_Result::Bad;
		}
	}

	void RemoteUserAction::Invoke(){		
		if (session_)
			session_->do_write(reinterpret_cast<char*>(original_data_.data()), original_data_.size());
	}


	void RemoteUserAction::Reset(){
		original_data_.clear();
		length_ = 0;
	}

}