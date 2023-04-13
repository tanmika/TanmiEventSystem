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

	class EventSystem
	{
	private:
		// ���ݳ�Ա
		std::multimap<EventID, std::weak_ptr<Listener>> EventList;
		std::vector<std::shared_ptr<Listener>> listenersToWake;
		std::vector<std::function<bool()>> eventsPreprocess;
		std::mutex mtx;
		std::mutex mtx_temp;
	private:
		// ����ϵͳ
		std::shared_ptr<MessageHandler> messageHandler;
		bool isMessageHandlerRegisted = false;
		std::shared_ptr<MessageHandlerUpdate> messageHandlerUpdate;
		bool isMessageHandlerUpdateRegisted = false;
	private:
		// ���캯��
		EventSystem()
		{
			eventsPreprocess.emplace_back();
		}
		EventSystem(const EventSystem&) = delete;
		EventSystem& operator=(const EventSystem&) = delete;
		EventSystem(EventSystem&&) = delete;
		EventSystem& operator=(EventSystem&&) = delete;
	public:
		// ��ȡEventSystemʵ������
		static EventSystem& Instance();

		// ע����Ϣ������
		void RegisterMessageHandler(std::shared_ptr<MessageHandler> _messageHandler);

		// ע����Ϣ������
		void RegisterMessageHandlerUpdate(std::shared_ptr<MessageHandlerUpdate> _messageHandlerUpdate);

		// ע���¼�
		void RegisterEvent(Event& event);

		// �����¼�
		void TriggerEvent(const Event& event);

		// �����¼�
		// event: �¼�����
		// ms: �¼�����ʱ�ĸ��¼��
		void TriggerEventUpdate(const Event& event, double ms);

		// ����¼�
		// event: �¼�����
		// client: ��������
		void AddEventHandler(const Event& event, std::shared_ptr<Listener> client);

		// �Ƴ��¼����ض�����
		// event: �¼�����
		// client: ���Ƴ��ļ�������
		void RemoveEventHandler(const Event& event, std::shared_ptr<Listener> client);

		// �Ƴ�ָ���¼������µ����м���
		// event: �¼�����
		void RemoveAllEventForEventName(const Event& event);

		// �Ƴ�������ָ������������ص��¼�
		// client: ���Ƴ��ļ�������
		void RemoveAllEventForListener(const std::shared_ptr<Listener> client);

		// �ж�ָ���¼��Ƿ����
		// event: �¼�����
		bool IsEventExist(const Event& event)const;

		// �ж�ָ���¼��Ƿ���ڣ����쳣���
		// event: �¼�����
		bool IsEventExistNoException(const Event& event)const;

		// ��������
		~EventSystem();
	};

	EventSystem& TanmiEngine::EventSystem::Instance()
	{
		static EventSystem instance;
		return instance;
	}

	inline void EventSystem::RegisterMessageHandler(std::shared_ptr<MessageHandler> _messageHandler)
	{
		messageHandler = _messageHandler;
		std::thread thread(&MessageHandler::Run, _messageHandler.get());
		thread.detach();
		isMessageHandlerRegisted = true;
	}

	inline void EventSystem::RegisterMessageHandlerUpdate(std::shared_ptr<MessageHandlerUpdate> _messageHandlerUpdate)
	{
		messageHandlerUpdate = _messageHandlerUpdate;
		std::thread thread(&MessageHandlerUpdate::Run, _messageHandlerUpdate.get());
		thread.detach();
		isMessageHandlerUpdateRegisted = true;
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
	// Ч�ʵ�
	//
	inline void EventSystem::RemoveAllEventForListener(const std::shared_ptr<Listener> client)
	{
		try
		{
			std::lock_guard<std::mutex> lock(mtx);
			bool isExist = false;
			//1
			/*auto it = std::remove_if(EventList.begin(), EventList.end(),
				[&client](const std::multimap<EventID, std::weak_ptr<Listener>>::value_type& pair)
				{
					return pair.second.lock() && pair.second.lock() == client;
				});
			if (it != EventList.end())
			{
				EventList.erase(it, EventList.end());
				isExist = true;
			}*/
			//2
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
}