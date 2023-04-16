#pragma once
/*****************************************************************//**
 * \file   TanmiMessageQuene.hpp
 * \brief  事件队列
 * 
 * \author tanmika
 * \date   April 2023
 *********************************************************************/
#include <queue>
#include <mutex>
#include <condition_variable>
#include "TanmiEvent.hpp"

using EventID = int;
namespace TanmiEngine {
	/**
	* @brief 用于存储和处理类型T的消息队列
	*
	* @tparam T 队列中元素的类型
	*/
	template<typename T>
	class MessageQueue
	{
	public:
		/*
		* @brief 默认构造函数
		*/
		MessageQueue() = default;
		/**
		 * @brief 将消息T推入队列
		 *
		 * @param message 要推入队列的消息引用
		 */
		void Push(T& message)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(message);
			cv_.notify_one();
		}
		/**
		 * @brief 从队列中取出下一个消息T并将其弹出队列
		 *
		 * @return T 弹出的消息
		 */
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
		std::queue<T> queue_;	//< 存储元素的队列
		std::mutex mutex_;		//< 互斥锁
		std::condition_variable cv_;	//< 条件变量
	};
}