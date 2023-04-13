#pragma once

using EventID = int;
namespace TanmiEngine {
	class Listener
	{
	public:
		Listener() = default;
		virtual ~Listener() = default;
		virtual void WakeEvent(const EventID event) = 0;
		virtual void WakeEventUpdate(const EventID event, double ms) = 0;
	};
}