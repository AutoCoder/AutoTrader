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

ActionQueueProcessor::ActionQueueProcessor(TQueue& queue)
	: m_running(false)
	, m_queueRef(queue)
{
}

ActionQueueProcessor::~ActionQueueProcessor(){}

void ActionQueueProcessor::Start(){
	m_running = true;
	while (m_running){

		std::shared_ptr<Transmission::BaseAction> action = m_queueRef.Wait_And_Pop();
		
		action->Invoke();
	}
}

void ActionQueueProcessor::Stop(){
	m_queueRef.Push_back(LocalQuitAction(m_running));
}