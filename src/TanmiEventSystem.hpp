#pragma once
/*****************************************************************//**
 * \file   TanmiEventSystem.hpp
 * \brief  �첽�¼�����ϵͳ
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
	 * @brief EventSystem ���쳣���ࡣ
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
	* @brief  �¼�δ�ҵ��쳣
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
	 * @brief  ������δ�ҵ��쳣
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
	 * @brief	�¼�δע���쳣
	 */
	class EventSystemEventNotRegistedException : public EventSystemException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection event_not_regist.";
		}
	};

	// �����
	/**
	 * @brief �¼��������
	 */
	template<typename T>
	concept EventBase = std::is_base_of_v<Event, T>;

	/**
	 * @brief ��Ϣ�������������
	 */
	template<typename T>
	concept MessageHandlerBase = std::is_base_of_v<MessageHandler, T>;

	/**
	 * @brief ��Ϣ���������»������
	 */
	template<typename T>
	concept MessageHandlerUpdateBase = std::is_base_of_v<MessageHandlerUpdate, T>;

	/**
	 * @brief ��˳���ȡ����������
	 */
	template<typename Container>
	concept InputRangeContainer = requires(Container c)
	{
		requires std::ranges::input_range<Container>;
	};

	/**
	 * @brief ��˳���ȡ���¼���������
	 */
	template<typename Container>
	concept EventContainer = requires(Container c)
	{
		typename Container::value_type;
			requires std::is_base_of_v<Event, typename Container::value_type>;
			requires std::ranges::input_range<Container>;
	};

	/**
	 * @brief ��˳���ȡ�ļ�����ָ����������
	 */
	template<typename Container>
	concept ListenerSptrContainer = requires(Container c)
	{
		typename Container::value_type;
			requires std::derived_from<typename Container::value_type::element_type, Listener>;
			requires std::ranges::input_range<Container>;
	};

	/**
	 * @brief �¼�ϵͳ��
	 */
	class EventSystem
	{
	private:
		// ���ݳ�Ա
		std::multimap<EventID, std::weak_ptr<Listener>> EventList;	//< �¼��б�<�¼�ID, ������ָ��>
		std::vector<std::shared_ptr<Listener>> listenersToWake;		//< �����ѵļ������б�
		std::vector<std::function<bool()>> eventsPreprocess;		//< �¼�Ԥ�������б�
		std::mutex mtx;												//< ������
		std::mutex mtx_temp;										//< ��ʱ������
	private:
		// ����ϵͳ
		std::shared_ptr<MessageHandler> messageHandler;				//< ��Ϣ������
		bool isMessageHandlerRegisted = false;						//< �Ƿ�ע������Ϣ������
		std::shared_ptr<MessageHandlerUpdate> messageHandlerUpdate;	//< ��Ϣ������������ʱ�������
		bool isMessageHandlerUpdateRegisted = false;				//< �Ƿ�ע������Ϣ������������ʱ�������
	private:
		/**
		 * @brief ���캯����Ĭ��ռ��Ԥ�����б�0��λ
		 */
		EventSystem()
		{
			eventsPreprocess.emplace_back();
		}
		EventSystem(const EventSystem&) = delete;					//< ��ֹ��������
		EventSystem& operator=(const EventSystem&) = delete;		//< ��ֹ������ֵ
		EventSystem(EventSystem&&) = delete;						//< ��ֹ�ƶ�����
		EventSystem& operator=(EventSystem&&) = delete;				//< ��ֹ�ƶ���ֵ
	public:
		static EventSystem& Instance();								//< ��ȡEventSystemʵ������

		/**
		 * @brief ע����Ϣ������
		 * 
		 * @param _messageHandler ��Ϣ������
		 */
		void RegisterMessageHandler(std::shared_ptr<MessageHandler> _messageHandler);

		/**
		 * @brief ע�Ტ������Ϣ������
		 * 
		 * @tparam T ��Ϣ����������
		 * @param pram ��Ϣ�������������
		 * 
		 * @return ��Ϣ������ָ��
		 */
		template<MessageHandlerBase T, typename... P>
		std::shared_ptr<T> RegisterMessageHandler(P...pram);

		/**
		 * @brief ע����Ϣ������������ʱ�������
		 * 
		 * @param _messageHandlerUpdate ��Ϣ������������ʱ�������
		 */
		void RegisterMessageHandlerUpdate(std::shared_ptr<MessageHandlerUpdate> _messageHandlerUpdate);

		/**
		 * @brief ע�Ტ������Ϣ������������ʱ�������
		 * 
		 * @tparam T ��Ϣ����������
		 * @param pram ��Ϣ�������������
		 * 
		 * @return std::shared_ptr<T> ��Ϣ������ָ��
		 */
		template<MessageHandlerUpdateBase T, typename... P>
		std::shared_ptr<T> RegisterMessageHandlerUpdate(P...pram);

		/**
		 * @brief ʹ��Ĭ����Ϣ������
		 */
		void UseMessageHandlerDefault();

		/**
		 * @brief ע���¼�
		 * 
		 * @param event �¼�
		 */
		void RegisterEvent(Event& event);

		/**
		 * @brief ʹ������ע���¼�
		 * 
		 * @tparam T �¼���������
		 * @param events �¼�����
		 */
		template<EventContainer T>
		void RegisterEvent(T& events);

		/**
		 * @brief �½���ע���¼�
		 * 
		 * @tparam T �¼�����
		 * @return std::shared_ptr<T> �¼�ָ��
		 */
		template<EventBase T>
		std::shared_ptr<T> NewAndRegisterEvent();

		/**
		 * @brief �½���ע��ָ�������¼�
		 * 
		 * @tparam T �¼�����
		 * @param num �¼�����
		 * 
		 * @return std::vector<std::shared_ptr<T>> �¼�ָ���б�
		 */
		template<EventBase T>
		inline std::vector<std::shared_ptr<T>> NewAndRegisterEvents(int num);

		/**
		 * @brief �½���ע��ָ�����������¼�
		 * 
		 * @tparam T �¼�����
		 * @tparam C �¼���������
		 * 
		 * @param num �¼�����
		 * 
		 * @return C �¼�����
		 */
		template<EventBase T, InputRangeContainer C>
		C NewAndRegisterEvents(int num);

		/**
		 * @brief �����¼�
		 * 
		 * @param event �¼�
		 */
		void TriggerEvent(const Event& event);

		/**
		 * @brief �����¼�
		 * 
		 * @param eventID �¼�ID
		 */
		void TriggerEvent(const EventID eventID);

		/**
		 * @brief �����¼�
		 * 
		 * @param event �¼�
		 * @param ms �¼�����ʱ�ĸ��¼��
		 */
		void TriggerEventUpdate(const Event& event, double ms);

		/**
		 * @brief �����¼�
		 * 
		 * @param eventID �¼�ID
		 * @param ms �¼�����ʱ�ĸ��¼��
		 */
		void TriggerEventUpdate(const EventID eventID, double ms);

		/**
		 * @brief ����¼�
		 * 
		 * @param event �¼�����
		 * @param client ��������
		 */
		void AddEventHandler(const Event& event, std::shared_ptr<Listener> client);
		
		/**
		 * @brief ����¼�
		 * 
		 * @tparam T �¼���������
		 * 
		 * @param events �¼���������
		 * @param client ��������
		 */
		template<EventContainer T>
		void AddEventHandler(const T& events, std::shared_ptr<Listener> client);

		/**
		 * @brief ����¼�
		 * 
		 * @tparam T ����������������
		 * @param event �¼�����
		 * @param clients ������������
		 */
		template<ListenerSptrContainer T>
		void addEventHandler(const Event& event, const T& clients);

		/**
		 * @brief �Ƴ��¼����ض�����
		 * 
		 * @param event �¼�����
		 * @param client ���Ƴ��ļ�������
		 */
		void RemoveEventHandler(const Event& event, std::shared_ptr<Listener> client);

		/**
		 * @brief �Ƴ�ָ���¼������µ����м���
		 * 
		 * @param event �¼�����
		 */
		void RemoveAllEventForEventName(const Event& event);

		/**
		 * @brief �Ƴ�������ָ������������ص��¼�
		 * 
		 * @param client ���Ƴ��ļ�������
		 */
		void RemoveAllEventForListener(const std::shared_ptr<Listener> client);

		/**
		 * @brief �ж�ָ���¼��Ƿ����
		 * 
		 * @param event �¼�����
		 * @return  true ����
		 * @return  false ������
		 */
		bool IsEventExist(const Event& event)const;

		/**
		 * @brief �ж�ָ���¼��Ƿ���ڣ����쳣���
		 * 
		 * @param event �¼�����
		 * @return true ����
		 * @return false ������
		 */
		bool IsEventExistNoException(const Event& event)const;

		/**
		 * @brief ��������
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
			// Ѱ���¼�
			auto range = EventList.equal_range(event.ID);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			// Ԥ����
			if (eventsPreprocess[event.ID]() == false)
				return;
			// ��λ
			auto& it = range.first;
			auto& it_end = range.second;
			std::lock_guard<std::mutex> lock(mtx_temp);
			// ѹ����
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
			// ֪ͨ
			for (auto& listener : listenersToWake)
			{
				messageHandler->Post(event.ID, listener);
			}
			// ��ջ���
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
			// Ѱ���¼�
			auto range = EventList.equal_range(eventID);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			// Ԥ����
			if (eventsPreprocess[eventID]() == false)
				return;
			// ��λ
			auto& it = range.first;
			auto& it_end = range.second;
			std::lock_guard<std::mutex> lock(mtx_temp);
			// ѹ����
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
			// ֪ͨ
			for (auto& listener : listenersToWake)
			{
				messageHandler->Post(eventID, listener);
			}
			// ��ջ���
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
			// Ѱ���¼�
			auto range = EventList.equal_range(event.ID);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			// Ԥ����
			if (eventsPreprocess[event.ID]() == false)
				return;
			// ��λ
			auto& it = range.first;
			auto& it_end = range.second;
			std::lock_guard<std::mutex> lock(mtx_temp);
			// ѹ����
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
			// ֪ͨ
			for (auto& listener : listenersToWake)
			{
				messageHandlerUpdate->Post(event.ID, listener, ms);
			}
			// ��ջ���
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
			// Ѱ���¼�
			auto range = EventList.equal_range(eventID);
			if (std::distance(range.first, range.second) == 0)
			{
				throw EventSystemEventNotFoundException();
			}
			// Ԥ����
			if (eventsPreprocess[eventID]() == false)
				return;
			// ��λ
			auto& it = range.first;
			auto& it_end = range.second;
			std::lock_guard<std::mutex> lock(mtx_temp);
			// ѹ����
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
			// ֪ͨ
			for (auto& listener : listenersToWake)
			{
				messageHandlerUpdate->Post(eventID, listener, ms);
			}
			// ��ջ���
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
	// Ч�ʵ�
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