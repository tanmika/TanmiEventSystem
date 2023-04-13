#pragma once

#include "TanmiMessageQuene.hpp"
#include "TanmiListener.hpp"

namespace TanmiEngine
{
	class MessageHandler
	{
	public:
		MessageHandler() = default;
		~MessageHandler() = default;
		virtual void Post(EventID id, std::shared_ptr<Listener> cilent)
		{
			listener.Push(cilent);
			listener_id.Push(id);
		}

		virtual void Run()
		{
			while (!exit)
			{
				HandleMessage(listener_id.Pop(), std::move(listener.Pop()));
			}
		}

		virtual void HandleMessage(const EventID id, std::shared_ptr<Listener> message)
		{
			message->WakeEvent(id);
		}

		void Exit()
		{
			exit = true;
		}
	protected:
		MessageQueue<std::shared_ptr<Listener>> listener;
		MessageQueue<EventID> listener_id;
		bool exit = false;
	};

	class MessageHandlerUpdate
	{
	public:
		MessageHandlerUpdate()=default;
		~MessageHandlerUpdate()=default;
		virtual void Post(EventID id, std::shared_ptr<Listener> cilent, double ms)
		{
			listener.Push(cilent);
			listener_id.Push(id);
			listener_t.Push(ms);
		}

		virtual void Run()
		{
			while (!exit)
			{
				HandleMessage(listener_id.Pop(), std::move(listener.Pop()), listener_t.Pop());
			}
		}

		virtual void HandleMessage(const EventID id, std::shared_ptr<Listener> message, double time)
		{
			message->WakeEventUpdate(id, time);
		}

		void Exit()
		{
			exit = true;
		}
	protected:
		MessageQueue<std::shared_ptr<Listener>> listener;
		MessageQueue<EventID> listener_id;
		MessageQueue<double> listener_t;
		bool exit = false;
	};
}