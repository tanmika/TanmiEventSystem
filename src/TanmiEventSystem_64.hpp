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
		// ��ȡEventSystemʵ������
		static EventSystem& Instance();

		// �����¼�
		// event: �¼�����
		void TriggerEvent(const std::string& event);

		// �����¼�
		// event: �¼�����
		// ms: �¼�����ʱ�ĸ��¼��
		void TriggerEventUpdate(const std::string& event, double ms);

		// ����¼�
		// event: �¼�����
		// client: ��������
		void AddEventHandler(const std::string& event, std::shared_ptr<Listener> client);

		// �Ƴ��¼����ض�����
		// event: �¼�����
		// client: ���Ƴ��ļ�������
		void RemoveEventHandler(const std::string& event, std::shared_ptr<Listener> client);

		// �Ƴ�ָ���¼������µ����м���
		// event: �¼�����
		void RemoveAllEventForEventName(const std::string& event);

		// �Ƴ�������ָ������������ص��¼�
		// client: ���Ƴ��ļ�������
		void RemoveAllEventForListener(const std::shared_ptr<Listener> client);

		// �ж�ָ���¼��Ƿ����
		// event: �¼�����
		bool IsEventExist(const std::string& event)const;
		
		// �ж�ָ���¼��Ƿ���ڣ����쳣���
		// event: �¼�����
		bool IsEventExistNoException(const std::string& event)const;

		// ��������
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