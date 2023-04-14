//
//	Tanmika --2023.4.8
//
#pragma once
#include <map>
#include <memory>
#include <iostream>
#include <vector>
#include <functional>

#include "TanmiEventHandler.hpp"

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
	class EventSystemEventNotRegistedException : public EventSystemException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection event_not_regist.";
		}
	};

	// 事件类类型
	template<typename T>
	concept EventBase = std::is_base_of_v<Event, T>;

	// 消息处理器类类型
	template<typename T>
	concept MessageHandlerBase = std::is_base_of_v<MessageHandler, T>;

	// 消息处理器更新类类型
	template<typename T>
	concept MessageHandlerUpdateBase = std::is_base_of_v<MessageHandlerUpdate, T>;

	class EventSystem
	{
	private:
		// 数据成员
		std::multimap<EventID, std::weak_ptr<Listener>> EventList;
		std::vector<std::shared_ptr<Listener>> listenersToWake;
		std::vector<std::function<bool()>> eventsPreprocess;
		std::mutex mtx;
		std::mutex mtx_temp;
	private:
		// 其他系统
		std::shared_ptr<MessageHandler> messageHandler;
		bool isMessageHandlerRegisted = false;
		std::shared_ptr<MessageHandlerUpdate> messageHandlerUpdate;
		bool isMessageHandlerUpdateRegisted = false;
	private:
		// 构造函数
		EventSystem()
		{
			eventsPreprocess.emplace_back();
		}
		EventSystem(const EventSystem&) = delete;
		EventSystem& operator=(const EventSystem&) = delete;
		EventSystem(EventSystem&&) = delete;
		EventSystem& operator=(EventSystem&&) = delete;
	public:
		// 获取EventSystem实例引用
		static EventSystem& Instance();

		// 注册消息处理器
		void RegisterMessageHandler(std::shared_ptr<MessageHandler> _messageHandler);

		// 注册并返回消息处理器
		template<MessageHandlerBase T, typename... P>
		std::shared_ptr<T> RegisterMessageHandler(P...pram);

		// 注册消息处理器
		void RegisterMessageHandlerUpdate(std::shared_ptr<MessageHandlerUpdate> _messageHandlerUpdate);

		// 注册并返回消息处理器
		template<MessageHandlerUpdateBase T, typename... P>
		std::shared_ptr<T> RegisterMessageHandlerUpdate(P...pram);

		// 使用默认消息处理器
		void UseMessageHandlerDefault();

		// 注册事件
		void RegisterEvent(Event& event);

		// 新建并注册事件
		template<EventBase T>
		std::shared_ptr<T> NewAndRegisterEvent();

		// 触发事件
		void TriggerEvent(const Event& event);

		// 触发事件
		// event: 事件名称
		// ms: 事件触发时的更新间隔
		void TriggerEventUpdate(const Event& event, double ms);

		// 添加事件
		// event: 事件名称
		// client: 监听对象
		void AddEventHandler(const Event& event, std::shared_ptr<Listener> client);

		// 移除事件下特定监听
		// event: 事件名称
		// client: 待移除的监听对象
		void RemoveEventHandler(const Event& event, std::shared_ptr<Listener> client);

		// 移除指定事件名称下的所有监听
		// event: 事件名称
		void RemoveAllEventForEventName(const Event& event);

		// 移除所有与指定监听对象相关的事件
		// client: 待移除的监听对象
		void RemoveAllEventForListener(const std::shared_ptr<Listener> client);

		// 判断指定事件是否存在
		// event: 事件名称
		bool IsEventExist(const Event& event)const;

		// 判断指定事件是否存在，无异常检测
		// event: 事件名称
		bool IsEventExistNoException(const Event& event)const;

		// 析构函数
		~EventSystem();
	};

	EventSystem& TanmiEngine::EventSystem::Instance()
	{
		static EventSystem instance;
		return instance;
	}

	inline void EventSystem::RegisterMessageHandler(std::shared_ptr<MessageHandler> _messageHandler)
	{
		if (isMessageHandlerRegisted)
			messageHandler->Exit();
		messageHandler = _messageHandler;
		std::thread thread(&MessageHandler::Run, _messageHandler.get());
		thread.detach();
		isMessageHandlerRegisted = true;
	}

	inline void EventSystem::RegisterMessageHandlerUpdate(std::shared_ptr<MessageHandlerUpdate> _messageHandlerUpdate)
	{
		if (isMessageHandlerUpdateRegisted)
			messageHandlerUpdate->Exit();
		messageHandlerUpdate = _messageHandlerUpdate;
		std::thread thread(&MessageHandlerUpdate::Run, _messageHandlerUpdate.get());
		thread.detach();
		isMessageHandlerUpdateRegisted = true;
	}

	inline void EventSystem::UseMessageHandlerDefault()
	{
		auto messageHandler = RegisterMessageHandler<MessageHandler>();
		auto messageHandlerUpdate = RegisterMessageHandlerUpdate<MessageHandlerUpdate>();
	}

	inline void EventSystem::RegisterEvent(Event& event)
	{
		static int eventID = 1;
		auto f = std::bind(&Event::preProcess, event);
		eventsPreprocess.push_back(f);
		event.ID = eventID++;
	}

	inline void EventSystem::TriggerEvent(const Event& event)
	{
		try
		{
			// 寻找事件
			auto range = EventList.equal_range(event.ID);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			// 预处理
			if (eventsPreprocess[event.ID]() == false)
				return;
			// 定位
			auto& it = range.first;
			auto& it_end = range.second;
			std::lock_guard<std::mutex> lock(mtx_temp);
			// 压缓冲
			while (it != it_end)
			{
				if (!it->second.expired())
				{
					listenersToWake.push_back(it->second.lock());
					it++;
				}
				else
				{
					EventList.erase(it++);
				}
			}
			// 通知
			for (auto& listener : listenersToWake)
			{
				messageHandler->Post(event.ID, listener);
			}
			// 清空缓冲
			listenersToWake.clear();
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::WakeEvent()" << e.what() << std::endl;
		}
	}

	inline void EventSystem::TriggerEventUpdate(const Event& event, double ms)
	{
		try
		{
			// 寻找事件
			auto range = EventList.equal_range(event.ID);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			// 预处理
			if (eventsPreprocess[event.ID]() == false)
				return;
			// 定位
			auto& it = range.first;
			auto& it_end = range.second;
			std::lock_guard<std::mutex> lock(mtx_temp);
			// 压缓冲
			while (it != it_end)
			{
				if (!it->second.expired())
				{
					listenersToWake.push_back(it->second.lock());
					it++;
				}
				else
				{
					EventList.erase(it++);
				}
			}
			// 通知
			for (auto& listener : listenersToWake)
			{
				messageHandlerUpdate->Post(event.ID, listener, ms);
			}
			// 清空缓冲
			listenersToWake.clear();
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::TriggerEventUpdate()" << e.what() << std::endl;
		}
	}

	inline void EventSystem::AddEventHandler(const Event& event, std::shared_ptr<Listener> client)
	{
		std::lock_guard<std::mutex> lock(mtx);
		std::weak_ptr<Listener> _client = client;
		EventList.insert(std::make_pair(event.ID, std::move(_client)));
	}

	inline void EventSystem::RemoveEventHandler(const Event& event, std::shared_ptr<Listener> client)
	{
		try
		{
			if (EventList.contains(event.ID) == false)
			{
				throw EventSystemEventNotFoundException();
			}
			std::lock_guard<std::mutex> lock(mtx);
			auto range = EventList.equal_range(event.ID);
			auto it = std::find_if(range.first, range.second, [&client](const auto& _wptr)
				{
					return _wptr.second.lock() && _wptr.second.lock() == client;
				}
			);
			if (it != range.second)
			{
				EventList.erase(it);
			}
			else
			{
				throw EventSystemListenerNotFoundException();
			}
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::AddEventHandler()" << e.what() << std::endl;
		}
	}
	inline void EventSystem::RemoveAllEventForEventName(const Event& event)
	{
		try
		{
			if (EventList.contains(event.ID) == false)
			{
				throw EventSystemEventNotFoundException();
			}
			std::lock_guard<std::mutex> lock(mtx);
			EventList.erase(event.ID);
		}
		catch (EventSystemException& e)
		{
			std::cout << "::EventSystem::RemoveAllEventForEventName()" << e.what() << std::endl;
		}
	}
	//
	// 效率低
	//
	inline void EventSystem::RemoveAllEventForListener(const std::shared_ptr<Listener> client)
	{
		try
		{
			std::lock_guard<std::mutex> lock(mtx);
			bool isExist = false;
			auto it = EventList.begin();
			while (it != EventList.end())
			{
				if (it->second.lock() && it->second.lock() == client)
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
	inline bool EventSystem::IsEventExist(const Event& event) const
	{
		try
		{
			if (EventList.contains(event.ID))
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
	inline bool EventSystem::IsEventExistNoException(const Event& event) const
	{
		if (EventList.contains(event.ID))
		{
			return true;
		}
		return false;
	}

	inline EventSystem::~EventSystem()
	{
		if (isMessageHandlerRegisted)
		{
			messageHandler->Exit();
		}
		if (isMessageHandlerUpdateRegisted)
		{
			messageHandlerUpdate->Exit();
		}
	}

	template<MessageHandlerBase T, typename ...P>
	inline std::shared_ptr<T> EventSystem::RegisterMessageHandler(P ...pram)
	{
		std::shared_ptr<T> _messageHandler;
		if constexpr(sizeof ...(pram) == 0)
		{
			_messageHandler = std::make_shared<T>();
		}
		else
		{
			_messageHandler = std::make_shared<T>(std::forward<P>(pram)...);
		}

		if (isMessageHandlerRegisted)
			messageHandler->Exit();
		messageHandler = _messageHandler;

		std::thread thread(&MessageHandler::Run, _messageHandler.get());
		thread.detach();
		isMessageHandlerRegisted = true;
		return _messageHandler;
	}

	template<MessageHandlerUpdateBase T, typename ...P>
	inline std::shared_ptr<T> EventSystem::RegisterMessageHandlerUpdate(P ...pram)
	{
		std::shared_ptr<T> _messageHandlerUpdate;
		if constexpr(sizeof ...(pram) == 0)
		{
			_messageHandlerUpdate = std::make_shared<T>();
		}
		else
		{
			_messageHandlerUpdate = std::make_shared<T>(std::forward<P>(pram)...);
		}
		
		if(isMessageHandlerUpdateRegisted)
			messageHandlerUpdate->Exit();
		messageHandlerUpdate = _messageHandlerUpdate;

		std::thread thread(&MessageHandlerUpdate::Run, _messageHandlerUpdate.get());
		thread.detach();
		isMessageHandlerUpdateRegisted = true;
		return _messageHandlerUpdate;
	}

	template<EventBase T>
	std::shared_ptr<T> EventSystem::NewAndRegisterEvent()
	{
		std::shared_ptr<T> event = std::make_shared<T>();
		RegisterEvent(*event);
		return event;
	}
}