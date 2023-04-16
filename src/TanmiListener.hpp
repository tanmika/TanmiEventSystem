#pragma once
/*****************************************************************//**
 * \file   TanmiListener.hpp
 * \brief  监听器基类
 * 
 * \author tanmika
 * \date   April 2023
 *********************************************************************/
using EventID = int;
namespace TanmiEngine {
	/**
	 * @brief 监听器基类
	 */
	class Listener
	{
	public:
		/**
		 * @brief 默认构造函数
		 */
		Listener() = default;
		/**
		 * @brief 默认析构函数
		 */
		virtual ~Listener() = default;
		/**
		 * @brief 事件响应函数
		 * 
		 * @param event 事件ID
		 */
		virtual void WakeEvent(const EventID event) = 0;
        /* @brief 事件响应函数
		 * 
		 * @param event 事件ID
		 * @param ms    事件发生所经过的时间（以毫秒为单位）
		 */
		virtual void WakeEventUpdate(const EventID event, double ms) = 0;
	};
}