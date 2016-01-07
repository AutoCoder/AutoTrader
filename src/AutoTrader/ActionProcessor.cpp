#include "stdafx.h"
#include "ActionProcessor.h"
#include "OrderQueue.h"
#include "fifo_action_queue.h"



void ProcessActionQueue(){
	
	while (true){

		std::shared_ptr<Transmission::BaseAction> action = Transmission::GetFIFOActionQueue().Wait_And_Pop();
		
		action->Invoke();
	}
}