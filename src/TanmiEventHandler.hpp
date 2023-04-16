#pragma once
/*****************************************************************//**
 * \file   TanmiEventHandler.hpp
 * \brief  异步事件处理器
 * 
 * \author tanmika
 * \date   April 2023
 *********************************************************************/
#include "TanmiMessageQuene.hpp"
#include "TanmiListener.hpp"

namespace TanmiEngine
{
	/**
	 * @brief 事件处理基类
	 */
	class MessageHandler
	{
	public:
		MessageHandler() = default;		//<	默认构造函数
		~MessageHandler() = default;	//<	默认析构函数
		/**
		* @brief 发送事件
		* 
		* @param id 事件ID
		* @param cilent 监听器
		*/
		virtual void Post(EventID id, std::shared_ptr<Listener> cilent)
		{
			listener.Push(cilent);
			listener_id.Push(id);
		}
		/**
		 * @brief 处理消息队列
		 */
		virtual void Run()
		{
			while (!exit)
			{
				HandleMessage(listener_id.Pop(), std::move(listener.Pop()));
			}
		}
		/**
		* @brief 处理消息
		* 
		* @param id 事件ID
		* @param message 监听器
		*/
		virtual void HandleMessage(const EventID id, std::shared_ptr<Listener> message)
		{
			message->WakeEvent(id);
		}
		/**
		* @brief 关闭消息处理器
		*/
		void Exit()
		{
			exit = true;
		}
	protected:
		MessageQueue<std::shared_ptr<Listener>> listener;	///< 监听器队列
		MessageQueue<EventID> listener_id;					///< 事件ID队列
		bool exit = false;									///< 是否退出
	};
	/**
	 * @brief 事件处理基类（带有时间参数）
	 */
	class MessageHandlerUpdate
	{
	public:
		MessageHandlerUpdate() = default;	//<	默认构造函数
		~MessageHandlerUpdate() = default;	//<	默认析构函数
		/**
		* @brief 发送事件
		* 
		* @param id 事件ID
		* @param cilent 监听器
		* @param ms 事件发生所经过的时间（以毫秒为单位）
		*/
		virtual void Post(EventID id, std::shared_ptr<Listener> cilent, double ms)
		{
			listener.Push(cilent);
			listener_id.Push(id);
			listener_t.Push(ms);
		}
		/**
		* @brief 处理消息队列
		*/
		virtual void Run()
		{
			while (!exit)
			{
				HandleMessage(listener_id.Pop(), std::move(listener.Pop()), listener_t.Pop());
			}
		}
        /* @brief 处理消息
		* 
		* @param id 事件ID
		* @param message 监听器
		* @param ms 事件发生所经过的时间（以毫秒为单位）
		*/
		virtual void HandleMessage(const EventID id, std::shared_ptr<Listener> message, double time)
		{
			message->WakeEventUpdate(id, time);
		}
		/**
		 * @brief 关闭消息处理器
		 */
		void Exit()
		{
			exit = true;
		}
	protected:
		MessageQueue<std::shared_ptr<Listener>> listener;	//<	监听器队列
		MessageQueue<EventID> listener_id;					//<	事件ID队列
		MessageQueue<double> listener_t;					//<	事件发生所经过的时间（以毫秒为单位）
		bool exit = false;									//<	是否退出
	};
}