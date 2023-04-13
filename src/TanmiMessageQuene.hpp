#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "TanmiEvent.hpp"

using EventID = int;
namespace TanmiEngine {
	template<typename T>
	class MessageQueue
	{
	public:
		MessageQueue() = default;

		void Push(T& message)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(message);
			cv_.notify_one();
		}

		T Pop()
		{
			std::unique_lock<std::mutex> lock(mutex_);
			cv_.wait(lock, [this]()
				{
					return !queue_.empty();
				});
			T message = std::move(queue_.front());
			queue_.pop();
			return message;
		}

	private:
		std::queue<T> queue_;
		std::mutex mutex_;
		std::condition_variable cv_;
	};
}