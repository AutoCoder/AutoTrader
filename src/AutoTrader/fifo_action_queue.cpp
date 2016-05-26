#include "stdafx.h"
#include "fifo_action_queue.h"

namespace Transmission{

	FIFOActionQueue& GetRequestActionQueue(){
		static FIFOActionQueue queue; //Scott Meyers singleton 
		return queue;
	}


	FIFOActionQueue& GetResponseActionQueue(){
		static FIFOActionQueue queue; //Scott Meyers singleton 
		return queue;
	}
}