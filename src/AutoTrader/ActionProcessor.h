#ifndef ACTION_QUEUE_PROCESSOR 
#define ACTION_QUEUE_PROCESSOR

namespace Transmission{
	class FIFOActionQueue;
};

class ActionQueueProcessor
{
public:
	typedef Transmission::FIFOActionQueue 	TQueue;
	ActionQueueProcessor(TQueue& queue);
	~ActionQueueProcessor();
	ActionQueueProcessor(const ActionQueueProcessor& processor) = delete;
	ActionQueueProcessor& operator = (const ActionQueueProcessor& processor) = delete;
	void Start();
	void Stop();

private:
	bool 									m_running;
	TQueue&    								m_queueRef;
};

#endif