#include "stdafx.h"
#include "fifo_action_queue.h"

namespace Transmission{

	FIFOActionQueue& GetFIFOActionQueue(){
		static FIFOActionQueue queue; //Scott Meyers singleton 
		return queue;
	}
}