#pragma once
/*****************************************************************//**
 * \file   TanmiEventSystem.hpp
 * \brief  异步事件处理系统
 * 
 * \author tanmi
 * \email tanmika@foxmail.com
 * \date   April 2023
 *********************************************************************/
#include <map>
#include <memory>
#include <iostream>
#include <vector>
#include <functional>

#include "TanmiMessageHandler.hpp"

#define EVENT_SYSTEM
namespace TanmiEngine
{
	/**
	 * @brief EventSystem 类异常基类。
	 */
	class EventSystemException : public std::exception
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection Event System Exception basic";
		}
	};
	/**
	* @brief  事件未找到异常
	*/
	class EventSystemEventNotFoundException : public EventSystemException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection event_not_found.";
		}
	};
	/**
	 * @brief  监听器未找到异常
	 */
	class EventSystemListenerNotFoundException : public EventSystemException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection listener_not_found.";
		}
	};
	/**
	 * @brief	事件未注册异常
	 */
	class EventSystemEventNotRegistedException : public EventSystemException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection event_not_regist.";
		}
	};

	// 概念定义
	/**
	 * @brief 事件基类概念
	 */
	template<typename T>
	concept EventBase = std::is_base_of_v<Event, T>;

	/**
	 * @brief 消息处理器基类概念
	 */
	template<typename T>
	concept MessageHandlerBase = std::is_base_of_v<MessageHandler, T>;

	/**
	 * @brief 消息处理器更新基类概念
	 */
	template<typename T>
	concept MessageHandlerUpdateBase = std::is_base_of_v<MessageHandlerUpdate, T>;

	/**
	 * @brief 可顺序读取的容器概念
	 */
	template<typename Container>
	concept InputRangeContainer = requires(Container c)
	{
		requires std::ranges::input_range<Container>;
	};

	/**
	 * @brief 可顺序读取的事件容器概念
	 */
	template<typename Container>
	concept EventContainer = requires(Container c)
	{
		typename Container::value_type;
			requires std::is_base_of_v<Event, typename Container::value_type>;
			requires std::ranges::input_range<Container>;
	};

	/**
	 * @brief 可顺序读取的监听器指针容器概念
	 */
	template<typename Container>
	concept ListenerSptrContainer = requires(Container c)
	{
		typename Container::value_type;
			requires std::derived_from<typename Container::value_type::element_type, Listener>;
			requires std::ranges::input_range<Container>;
	};

	/**
	 * @brief 事件系统类
	 */
	class EventSystem
	{
	private:
		// 数据成员
		std::multimap<EventID, std::weak_ptr<Listener>> EventList;	//< 事件列表，<事件ID, 监听器指针>
		std::vector<std::shared_ptr<Listener>> listenersToWake;		//< 待唤醒的监听器列表
		std::vector<std::function<bool()>> eventsPreprocess;		//< 事件预处理函数列表
		std::mutex mtx;												//< 互斥锁
		std::mutex mtx_temp;										//< 临时互斥锁
	private:
		// 其他系统
		std::shared_ptr<MessageHandler> messageHandler;				//< 消息处理器
		bool isMessageHandlerRegisted = false;						//< 是否注册了消息处理器
		std::shared_ptr<MessageHandlerUpdate> messageHandlerUpdate;	//< 消息处理器（带有时间参数）
		bool isMessageHandlerUpdateRegisted = false;				//< 是否注册了消息处理器（带有时间参数）
	private:
		/**
		 * @brief 构造函数，默认占据预处理列表0号位
		 */
		EventSystem()
		{
			eventsPreprocess.emplace_back();
		}
		EventSystem(const EventSystem&) = delete;					//< 禁止拷贝构造
		EventSystem& operator=(const EventSystem&) = delete;		//< 禁止拷贝赋值
		EventSystem(EventSystem&&) = delete;						//< 禁止移动构造
		EventSystem& operator=(EventSystem&&) = delete;				//< 禁止移动赋值
	public:
		static EventSystem& Instance();								//< 获取EventSystem实例引用

		/**
		 * @brief 注册消息处理器
		 * 
		 * @param _messageHandler 消息处理器
		 */
		void RegisterMessageHandler(std::shared_ptr<MessageHandler> _messageHandler);

		/**
		 * @brief 注册并返回消息处理器
		 * 
		 * @tparam T 消息处理器类型
		 * @param pram 消息处理器构造参数
		 * 
		 * @return 消息处理器指针
		 */
		template<MessageHandlerBase T, typename... P>
		std::shared_ptr<T> RegisterMessageHandler(P...pram);

		/**
		 * @brief 注册消息处理器（带有时间参数）
		 * 
		 * @param _messageHandlerUpdate 消息处理器（带有时间参数）
		 */
		void RegisterMessageHandlerUpdate(std::shared_ptr<MessageHandlerUpdate> _messageHandlerUpdate);

		/**
		 * @brief 注册并返回消息处理器（带有时间参数）
		 * 
		 * @tparam T 消息处理器类型
		 * @param pram 消息处理器构造参数
		 * 
		 * @return std::shared_ptr<T> 消息处理器指针
		 */
		template<MessageHandlerUpdateBase T, typename... P>
		std::shared_ptr<T> RegisterMessageHandlerUpdate(P...pram);

		/**
		 * @brief 使用默认消息处理器
		 */
		void UseMessageHandlerDefault();

		/**
		 * @brief 注册事件
		 * 
		 * @param event 事件
		 */
		void RegisterEvent(Event& event);

		/**
		 * @brief 使用容器注册事件
		 * 
		 * @tparam T 事件容器类型
		 * @param events 事件容器
		 */
		template<EventContainer T>
		void RegisterEvent(T& events);

		/**
		 * @brief 新建并注册事件
		 * 
		 * @tparam T 事件类型
		 * @return std::shared_ptr<T> 事件指针
		 */
		template<EventBase T>
		std::shared_ptr<T> NewAndRegisterEvent();

		/**
		 * @brief 新建并注册指定数量事件
		 * 
		 * @tparam T 事件类型
		 * @param num 事件数量
		 * 
		 * @return std::vector<std::shared_ptr<T>> 事件指针列表
		 */
		template<EventBase T>
		inline std::vector<std::shared_ptr<T>> NewAndRegisterEvents(int num);

		/**
		 * @brief 新建并注册指定类型数量事件
		 * 
		 * @tparam T 事件类型
		 * @tparam C 事件容器类型
		 * 
		 * @param num 事件数量
		 * 
		 * @return C 事件容器
		 */
		template<EventBase T, InputRangeContainer C>
		C NewAndRegisterEvents(int num);

		/**
		 * @brief 触发事件
		 * 
		 * @param event 事件
		 */
		void TriggerEvent(const Event& event);

		/**
		 * @brief 触发事件
		 * 
		 * @param eventID 事件ID
		 */
		void TriggerEvent(const EventID eventID);

		/**
		 * @brief 触发事件
		 * 
		 * @param event 事件
		 * @param ms 事件触发时的更新间隔
		 */
		void TriggerEventUpdate(const Event& event, double ms);

		/**
		 * @brief 触发事件
		 * 
		 * @param eventID 事件ID
		 * @param ms 事件触发时的更新间隔
		 */
		void TriggerEventUpdate(const EventID eventID, double ms);

		/**
		 * @brief 添加事件
		 * 
		 * @param event 事件名称
		 * @param client 监听对象
		 */
		void AddEventHandler(const Event& event, std::shared_ptr<Listener> client);
		
		/**
		 * @brief 添加事件
		 * 
		 * @tparam T 事件容器类型
		 * 
		 * @param events 事件名称容器
		 * @param client 监听对象
		 */
		template<EventContainer T>
		void AddEventHandler(const T& events, std::shared_ptr<Listener> client);

		/**
		 * @brief 添加事件
		 * 
		 * @tparam T 监听对象容器类型
		 * @param event 事件名称
		 * @param clients 监听对象容器
		 */
		template<ListenerSptrContainer T>
		void addEventHandler(const Event& event, const T& clients);

		/**
		 * @brief 移除事件下特定监听
		 * 
		 * @param event 事件名称
		 * @param client 待移除的监听对象
		 */
		void RemoveEventHandler(const Event& event, std::shared_ptr<Listener> client);

		/**
		 * @brief 移除指定事件名称下的所有监听
		 * 
		 * @param event 事件名称
		 */
		void RemoveAllEventForEventName(const Event& event);

		/**
		 * @brief 移除所有与指定监听对象相关的事件
		 * 
		 * @param client 待移除的监听对象
		 */
		void RemoveAllEventForListener(const std::shared_ptr<Listener> client);

		/**
		 * @brief 判断指定事件是否存在
		 * 
		 * @param event 事件名称
		 * @return  true 存在
		 * @return  false 不存在
		 */
		bool IsEventExist(const Event& event)const;

		/**
		 * @brief 判断指定事件是否存在，无异常检测
		 * 
		 * @param event 事件名称
		 * @return true 存在
		 * @return false 不存在
		 */
		bool IsEventExistNoException(const Event& event)const;

		/**
		 * @brief 析构函数
		 */
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

	template<EventBase T>
	std::shared_ptr<T> EventSystem::NewAndRegisterEvent()
	{
		std::shared_ptr<T> event = std::make_shared<T>();
		RegisterEvent(*event);
		return event;
	}

	template<EventBase T>
	inline std::vector<std::shared_ptr<T>> EventSystem::NewAndRegisterEvents(int num)
	{
		std::vector<std::shared_ptr<T>> container;
		while (num--)
		{
			std::shared_ptr<T> event = std::make_shared<T>();
			RegisterEvent(*event);
			container.push_back(event);
		}
		return container;
	}

	template<EventBase T, InputRangeContainer C>
	inline C EventSystem::NewAndRegisterEvents(int num)
	{
		C container;
		while (num--)
		{
			std::shared_ptr<T> event = std::make_shared<T>();
			RegisterEvent(*event);
			container.push_back(event);
		}
		return container;
	}

	inline void EventSystem::RegisterEvent(Event& event)
	{
		static int eventID = 1;
		auto f = std::bind(&Event::preProcess, event);
		eventsPreprocess.push_back(f);
		event.ID = eventID++;
	}

	template<EventContainer T>
	inline void EventSystem::RegisterEvent(T& events)
	{
		for (auto& event : events)
		{
			RegisterEvent(event);
		}
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

	inline void EventSystem::TriggerEvent(const EventID eventID)
	{
		try
		{
			// 寻找事件
			auto range = EventList.equal_range(eventID);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			// 预处理
			if (eventsPreprocess[eventID]() == false)
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
				messageHandler->Post(eventID, listener);
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

	inline void EventSystem::TriggerEventUpdate(const EventID eventID, double ms)
	{
		try
		{
			// 寻找事件
			auto range = EventList.equal_range(eventID);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			// 预处理
			if (eventsPreprocess[eventID]() == false)
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
				messageHandlerUpdate->Post(eventID, listener, ms);
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

	template<EventContainer T>
	inline void EventSystem::AddEventHandler(const T& events, std::shared_ptr<Listener> client)
	{
		std::lock_guard<std::mutex> lock(mtx);
		std::weak_ptr<Listener> _client = client;
		for (auto e : events)
		{
			EventList.insert(std::make_pair(e.ID, _client));
		}
	}

	template<ListenerSptrContainer T>
	inline void EventSystem::addEventHandler(const Event& event, const T& clients)
	{
		std::lock_guard<std::mutex> lock(mtx);
		for (auto e : clients)
		{
			std::weak_ptr<Listener> _client = e;
			EventList.insert(std::make_pair(event.ID, std::move(_client)));
		}
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
		if constexpr (sizeof ...(pram) == 0)
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
		if constexpr (sizeof ...(pram) == 0)
		{
			_messageHandlerUpdate = std::make_shared<T>();
		}
		else
		{
			_messageHandlerUpdate = std::make_shared<T>(std::forward<P>(pram)...);
		}

		if (isMessageHandlerUpdateRegisted)
			messageHandlerUpdate->Exit();
		messageHandlerUpdate = _messageHandlerUpdate;

		std::thread thread(&MessageHandlerUpdate::Run, _messageHandlerUpdate.get());
		thread.detach();
		isMessageHandlerUpdateRegisted = true;
		return _messageHandlerUpdate;
	}
}