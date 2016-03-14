#include "stdafx.h"
#include <atomic>
#include "ActionProcessor.h"
#include "fifo_action_queue.h"

class LocalQuitAction
{
public:
	LocalQuitAction(bool& running) : m_running(running) {}

	~LocalQuitAction(){}

	void Invoke(){ m_running = false; }

private:
	bool& m_running;
};

ActionQueueProcessor::ActionQueueProcessor()
	:m_running(false)
{

}

ActionQueueProcessor::~ActionQueueProcessor(){}

void ActionQueueProcessor::Start(){
	m_running = true;
	while (m_running){

		std::shared_ptr<Transmission::BaseAction> action = Transmission::GetFIFOActionQueue().Wait_And_Pop();
		
		action->Invoke();
	}
}

void ActionQueueProcessor::Stop(){
	Transmission::GetFIFOActionQueue().Push_back(LocalQuitAction(m_running));
}

ActionQueueProcessor& ActionQueueProcessor::Instance(){
	static ActionQueueProcessor instance;
	return instance;
}
