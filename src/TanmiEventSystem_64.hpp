//
//	Tanmika --2023.4.8
//
#pragma once
#include <mutex>
#include <map>
#include <memory>
#include <iostream>
#include <vector>
#include "TanmiListener_64.hpp"

#define EVENT_SYSTEM
namespace TanmiEngine
{
	// exception
	class EventSystemException : public std::exception
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection Event System Exception basic";
		}
	};
	class EventSystemEventNotFoundException : public EventSystemException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection event_not_found.";
		}
	};
	class EventSystemListenerNotFoundException : public EventSystemException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection listener_not_found.";
		}
	};
	class EventSystemNameExistException : public EventSystemException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection event_exist.";
		}
	};

	class EventSystem
	{
	private:
		std::multimap<std::string, std::shared_ptr<Listener>> EventList;
		std::vector<std::shared_ptr<Listener>> listenersToWake;
		std::mutex mtx;
		std::mutex mtx_temp;
	private:
		EventSystem() = default;
		EventSystem(const EventSystem&) = delete;
		EventSystem& operator=(const EventSystem&) = delete;
		EventSystem(EventSystem&&) = delete;
		EventSystem& operator=(EventSystem&&) = delete;
	public:
		// 获取EventSystem实例引用
		static EventSystem& Instance();

		// 触发事件
		// event: 事件名称
		void TriggerEvent(const std::string& event);

		// 触发事件
		// event: 事件名称
		// ms: 事件触发时的更新间隔
		void TriggerEventUpdate(const std::string& event, double ms);

		// 添加事件
		// event: 事件名称
		// client: 监听对象
		void AddEventHandler(const std::string& event, std::shared_ptr<Listener> client);

		// 移除事件下特定监听
		// event: 事件名称
		// client: 待移除的监听对象
		void RemoveEventHandler(const std::string& event, std::shared_ptr<Listener> client);

		// 移除指定事件名称下的所有监听
		// event: 事件名称
		void RemoveAllEventForEventName(const std::string& event);

		// 移除所有与指定监听对象相关的事件
		// client: 待移除的监听对象
		void RemoveAllEventForListener(const std::shared_ptr<Listener> client);

		// 判断指定事件是否存在
		// event: 事件名称
		bool IsEventExist(const std::string& event)const;
		
		// 判断指定事件是否存在，无异常检测
		// event: 事件名称
		bool IsEventExistNoException(const std::string& event)const;

		// 析构函数
		~EventSystem() = default;
	};

	EventSystem& TanmiEngine::EventSystem::Instance()
	{
		static EventSystem instance;
		return instance;
	}

	inline void EventSystem::TriggerEvent(const std::string& event)
	{
		try
		{
			auto range = EventList.equal_range(event);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			auto& it = range.first;
			auto& it_end = range.second;
			std::lock_guard<std::mutex> lock(mtx_temp);
			while (it != it_end)
			{
				listenersToWake.push_back(it->second);
				it++;
			}
			for (auto& listener : listenersToWake)
			{
				listener->WakeEvent(event);
			}
			listenersToWake.clear();
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::WakeEvent()" << e.what() << std::endl;
		}
	}

	inline void EventSystem::TriggerEventUpdate(const std::string& event, double ms)
	{
		try
		{
			auto range = EventList.equal_range(event);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			auto& it = range.first;
			auto& it_end = range.second;
			std::lock_guard<std::mutex> lock(mtx_temp);
			while (it != it_end)
			{
				listenersToWake.push_back(it->second);
				it++;
			}
			for (auto& listener : listenersToWake)
			{
				listener->WakeEventUpdate(event, ms);
			}
			listenersToWake.clear();
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::TriggerEventUpdate()" << e.what() << std::endl;
		}
	}

	inline void EventSystem::AddEventHandler(const std::string& event, std::shared_ptr<Listener> client)
	{
		std::lock_guard<std::mutex> lock(mtx);
		EventList.insert(std::make_pair(event, client));
	}

	inline void EventSystem::RemoveEventHandler(const std::string& event, std::shared_ptr<Listener> client)
	{
		try
		{
			if (EventList.contains(event) == false)
			{
				throw EventSystemEventNotFoundException();
			}
			std::lock_guard<std::mutex> lock(mtx);
			auto range = EventList.equal_range(event);
			auto& it = range.first;
			for (it = range.first; it != range.second; ++it)
			{
				if (it->second == client)
				{
					EventList.erase(it);
					break;
				}
			}
			if (it == range.second)
			{
				throw EventSystemListenerNotFoundException();
			}
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::AddEventHandler()" << e.what() << std::endl;
		}
	}
	inline void EventSystem::RemoveAllEventForEventName(const std::string& event)
	{
		try
		{
			if (EventList.contains(event) == false)
			{
				throw EventSystemEventNotFoundException();
			}
			std::lock_guard<std::mutex> lock(mtx);
			EventList.erase(event);
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::RemoveAllEventForEventName()" << e.what() << std::endl;
		}
	}
	inline void EventSystem::RemoveAllEventForListener(const std::shared_ptr<Listener> client)
	{
		try
		{
			std::lock_guard<std::mutex> lock(mtx);
			bool isExist = false;
			auto it = EventList.begin();
			while (it != EventList.end())
			{
				if (it->second == client)
				{
					it = EventList.erase(it);
					isExist = true;
				}
				else
				{
					++it;
				}
			}
			if (isExist == false)
			{
				throw EventSystemListenerNotFoundException();
			}
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::RemoveAllEventForListener()" << e.what() << std::endl;
		}
	}
	inline bool EventSystem::IsEventExist(const std::string& event) const
	{
		try
		{
			if (EventList.contains(event))
			{
				return true;
			}
			else
			{
				throw EventSystemEventNotFoundException();
			}
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::IsEventExist()" << e.what() << std::endl;
		}
		return false;
	}
	inline bool EventSystem::IsEventExistNoException(const std::string& event) const
	{
		if (EventList.contains(event))
		{
			return true;
		}
		return false;
	}
}