//
//	Tanmika --2023.4.8
//
#pragma once
#include <string>

namespace TanmiEngine {
	// 监听器接口
	class Listener
	{
	public:
		Listener() = default;
		~Listener() = default;
		// 事件触发
		// event:事件名称
		virtual void WakeEvent(const std::string& event) = 0;
		// 事件触发
		// event:事件名称
		// ms:触发时间间隔
		virtual void WakeEventUpdate(const std::string& event, double ms) = 0;
	};
}