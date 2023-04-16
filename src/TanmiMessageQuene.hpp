#pragma once
/*****************************************************************//**
 * \file   TanmiMessageQuene.hpp
 * \brief  �¼�����
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
	* @brief ���ڴ洢�ʹ�������T����Ϣ����
	*
	* @tparam T ������Ԫ�ص�����
	*/
	template<typename T>
	class MessageQueue
	{
	public:
		/*
		* @brief Ĭ�Ϲ��캯��
		*/
		MessageQueue() = default;
		/**
		 * @brief ����ϢT�������
		 *
		 * @param message Ҫ������е���Ϣ����
		 */
		void Push(T& message)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push(message);
			cv_.notify_one();
		}
		/**
		 * @brief �Ӷ�����ȡ����һ����ϢT�����䵯������
		 *
		 * @return T ��������Ϣ
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
		std::queue<T> queue_;	//< �洢Ԫ�صĶ���
		std::mutex mutex_;		//< ������
		std::condition_variable cv_;	//< ��������
	};
}