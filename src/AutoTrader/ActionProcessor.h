#ifndef ACTION_QUEUE_PROCESSOR 
#define ACTION_QUEUE_PROCESSOR

class ActionQueueProcessor
{
public:
	ActionQueueProcessor();
	~ActionQueueProcessor();
	ActionQueueProcessor(const ActionQueueProcessor& mgr) = delete;
	ActionQueueProcessor& operator = (const ActionQueueProcessor& mgr) = delete;
	void Start();
	void Stop();
	static ActionQueueProcessor& Instance();

private:
	bool m_running;
};

#endif