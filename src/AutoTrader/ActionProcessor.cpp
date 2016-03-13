#include "stdafx.h"
#include <atomic>
#include "ActionProcessor.h"
#include "fifo_action_queue.h"

extern std::atomic<bool> g_quit;

bool ProcessActionQueue(){
	
	while (!g_quit){

		std::shared_ptr<Transmission::BaseAction> action = Transmission::GetFIFOActionQueue().Wait_And_Pop();
		
		action->Invoke();
	}

	return true;
}