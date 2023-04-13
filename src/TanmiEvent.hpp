#pragma once

using EventID = int;
namespace TanmiEngine {
	class Event
	{
	public:
		Event() :ID(0)
		{}
		~Event() = default;

		virtual bool preProcess()
		{
			return true;
		}
		EventID ID;
	};
}