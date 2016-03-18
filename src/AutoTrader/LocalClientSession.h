#ifndef CLIENT_LOCAL_H
#define CLIENT_LOCAL_H

//#define FAKE_MD

#include <string>
#include "BaseClientSession.h"

class TickWrapper;


class LocalClientSession : public BaseClientSession
{
public:
	LocalClientSession(const std::string& userId);

	virtual void SendTickToClient(const TickWrapper& tick){};

	virtual ~LocalClientSession();

};

#endif