#pragma once
/*****************************************************************//**
 * \file   TanmiEvent.hpp
 * \brief  事件基类
 * 
 * \author tanmika
 * \date   April 2023
 *********************************************************************/
using EventID = int;
namespace TanmiEngine {
	/**
	 * @brief 事件基类
	 */
	class Event
	{
	public:
		/**
		 * @brief 构造函数，初始化事件ID为0
		 * 
		 */
		Event() :ID(0)
		{}
		/**
		* @brief 默认析构函数
		*/
		~Event() = default;
		/**
		 * @brief 预处理函数，用于实现预处理逻辑
		 *
		 * @return true 预处理成功，执行事件
		 * @return false 预处理失败，不执行事件
		 */
		virtual bool preProcess()
		{
			return true;
		}
		EventID ID;	///< 事件ID
	};
}