#ifndef FIFO_ACTION_QUEUE_H
#define FIFO_ACTION_QUEUE_H

#include "OrderQueue.h"

namespace Transmission{

	class BaseAction{
	public:
		virtual ~BaseAction(){}
		virtual void Invoke() = 0;
	};

	template<typename T>
	class Holder : public BaseAction
	{
	public:
		Holder(const T& other)
			: data_(other)
		{}

		virtual void Invoke(){
			data_.Invoke();
		}
	private:
		T data_;
	};

	class FIFOActionQueue
	{
	public:
		FIFOActionQueue(){}
		~FIFOActionQueue(){}
		FIFOActionQueue(const FIFOActionQueue& mgr) = delete;
		FIFOActionQueue& operator = (const FIFOActionQueue& mgr) = delete;

		template<typename T>
		void Push_back(T item)
		{
			std::shared_ptr<BaseAction> tem(std::make_shared<Holder<T> >(item));
			remote_action_queue.push(tem)
		}

		std::shared_ptr<BaseAction> Wait_And_Pop(){
			std::shared_ptr<BaseAction> value;
			remote_action_queue.wait_and_pop(value);
			return value;
		}

	private:
		threadsafe_queue<std::shared_ptr<BaseAction> > remote_action_queue;
	};

	FIFOActionQueue& GetFIFOActionQueue();
}


#endif