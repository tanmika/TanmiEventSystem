//
//	Tanmika --2023.4.2
//
#pragma once
#include <iostream>
#include <Windows.h>
#include <time.h>
#include <string>
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <exception>

#include"TanmiEventSystem_64.hpp"

// ʹ�õ���ģʽ���̰߳�ȫ ֧����չ
namespace TanmiEngine
{
	using ull = unsigned long long;
	using lint = LARGE_INTEGER;

	// exception
	class ClockException : public std::exception
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection Clock Exception basic";
		}
	};
	class ClockOutOfRangeException : public ClockException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection out_of_range";
		}
	};
	class ClockNotFoundException : public ClockException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection clock_not_found";
		}
	};
	class ClockNameExistException :public ClockException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection name_exist";
		}
	};
#ifdef EVENT_SYSTEM
	class ClockEventNotFoundException :public ClockException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection event_not_found";
		}
	};
#endif // EVENT_SYSTEM

	// element
	class ClockElem
	{
	public:
		~ClockElem() = default;
		ClockElem() = delete;
		ClockElem(ClockElem&, std::string, ull);
		ClockElem(std::string _name, ull _cycle, ull _update, float _scale, bool _pause) :
			name(_name), cycle(_cycle), last_cycle(_cycle), update_tick(_update), ins_cycle(_cycle),
			pause_cycle(0), relative_tick(0), scale(_scale), pause(_pause), temp_ull(0), temp_lint({})
		{}
		// cycle -> ���Լ��� ,tick -> ��Լ���
		std::string name;		// ����
		ull cycle;				// �ϴλ�����¼�����
		ull last_cycle;			// �ϴ���Ч����֡������
		ull update_tick;		// ��������
		ull ins_cycle;			// ��ʼʱ������
		ull pause_cycle;		// ��ͣʱ�̼�����
		ull relative_tick;		// ��Ծ������ܼ���
		float scale;			// ������
		bool pause;				// �Ƿ���ͣ
		ull temp_ull;			// ����
		lint temp_lint;			// ����
		std::mutex lock;		// ������
		std::mutex templock;	// ������
#ifdef EVENT_SYSTEM
		std::vector<std::string> eventList;	// �¼��б�
#endif // EVENT_SYSTEM
		//----------setting----------
		inline void setNameStr(const std::string _str)
		{
			std::lock_guard<std::mutex> elemguard(lock);
			name = _str;
		}
		inline void setFrameRateUll(const ull _rate)
		{
			std::lock_guard<std::mutex> elemguard(lock);
			update_tick = _rate;
		}
		inline void setScaleFloat(const float _scale)
		{
			std::lock_guard<std::mutex> elemguard(lock);
			scale = _scale;
		}
	};

	ClockElem::ClockElem(ClockElem& e, std::string _name, ull _cycle) : name(_name), cycle(_cycle), last_cycle(_cycle), ins_cycle(_cycle)
	{
		this->update_tick = e.update_tick;
		this->pause_cycle = e.pause_cycle;
		this->relative_tick = e.relative_tick;
		this->scale = e.scale;
		this->pause = e.pause;
		this->temp_ull = e.temp_ull;
		this->temp_lint = e.temp_lint;
	}

	// singleton class
	class Clock
	{
	private:
		std::unordered_map<std::string, std::shared_ptr<ClockElem>> clockMap;
		std::mutex lock_clk;
		ull temp_ull_clk;
		lint temp_lint_clk;
	private:
		const int MAX_FRAME_RATE_PER_SECOND = 1000;
		const double MIN_FRAME_RATE_PER_SECOND = 0.001;
		const int MAX_SCALE = 1000;
		const double MIN_SCALE = 0.001;
	private:
		Clock();
		Clock(Clock&) = delete;
		Clock& operator=(Clock&) = delete;
		Clock(Clock&&) = delete;
		Clock& operator=(Clock&&) = delete;
		//----------tool----------
		// ��ȡʱ�Ӷ���ָ�룬ʧ��ʱ����nullptr
		inline std::shared_ptr<ClockElem> getIterator(const std::string&);
		// ��ȡullʱ�Ӽ�����Ƶ�ʣ�ʹ�����ڻ���
		inline std::pair<ull, ull> getCycleAndFreqIns();
		// ��ȡullʱ�Ӽ�����ʹ��Ԫ�ػ���
		inline ull& getCycleNow(const std::string&);
		// ��ȡullʱ�Ӽ�����ʹ��Ԫ�ػ���
		inline ull& getCycleNow(std::shared_ptr<ClockElem>);
		// ��ȡullʱ��Ƶ�ʣ�ʹ��Ԫ�ػ���
		inline ull& getFreqNow(const std::string&);
		// ��ȡullʱ��Ƶ�ʣ�ʹ��Ԫ�ػ���
		inline ull& getFreqNow(std::shared_ptr<ClockElem>);

		//----------function----------
		// ��ȡʱ���Ƿ񳬹����µ㣬�������ʱ��
		inline bool isUpdate(const std::string&);
		// ��ȡʱ���Ƿ񳬹����µ㣬�������ʱ��
		inline bool isUpdate(std::shared_ptr<ClockElem>);

	public:
		// ��ȡClockʵ������
		static Clock& Instance();
		//----------elemFunction----------
		// �½�ʱ�ӣ�����Ϊʱ�����ƺ�ˢ����
		bool NewClock(const std::string&, double);
		// �Ƴ�ʱ�ӣ�����Ϊʱ������
		bool EraseClock(const std::string&);
		// ����ʱ�ӣ�����Ϊʱ�����ƺ���ʱ������
		bool CopyClock(const std::string&, const std::string&);
		//----------getFunction----------
		// ��ȡʱ���Ƿ񳬹����µ㣬�������ʱ�ӣ�����Ϊʱ������
		bool GetUpdate(const std::string&);
		// ��ȡʱ���Ƿ���ͣ������Ϊʱ������
		bool GetPause(const std::string&);
		// ��ȡʱ��ˢ���ʣ�����Ϊʱ������
		double GetFramePerSecond(const std::string&);
		// ��ȡʱ���Դ������������ľ���ʱ��������Ϊʱ������
		double GetElapsed(const std::string&);
		// ��ȡʱ������һˢ�½ڵ㾭���ľ���ʱ��������Ϊʱ������
		double GetTick(const std::string&);
		// ��ȡʱ���Դ����������������ʱ��������Ϊʱ������
		double GetElapsedRelative(const std::string&);
		// ��ȡʱ������һˢ�½ڵ㾭�������ʱ��������Ϊʱ������
		double GetTickRelative(const std::string&);
		//----------setFunction----------
		// ����ʱ����ͣ״̬������Ϊʱ�����ƺͲ���ֵ��true��ʾ��ͣ��false��ʾ����
		void SetPause(const std::string&, bool);
		// ����ˢ���ʣ�����Ϊʱ�����ƺ�ˢ����
		void SetFramePerSecond(const std::string&, double);
		// ����ʱ�����ţ�����Ϊʱ�����ƺ�����ֵ
		void SetFrameScale(const std::string&, double);
		// ����ʱ�Ӽ�ʱ��������Ϊʱ������
		void ResetClockIns(const std::string&);
		//-----------tool-----------
		// usingned long long��������ms������ת����Debug��
		inline int ull2ms_freq(ull time_ull)
		{
			auto freq = getCycleAndFreqIns().second;
			return (time_ull * 1000 / freq);
		}
#ifdef EVENT_SYSTEM
		// ����¼���ʱ��
		void AddEvent(const std::string&, const std::string&);
		// �Ƴ��¼�
		void RemoveEvent(const std::string&, const std::string&);
		// ��ȡ�¼��б�
		std::vector<std::string>& GetEventList(const std::string&);
		// ����¼��б�
		void ClearEventList(const std::string&);
#endif // EVENT_SYSTEM
		void DEBUG(const std::string&);
		~Clock() = default;
	};

	Clock& Clock::Instance()
	{
		static Clock clk;
		return clk;
	}

	inline bool Clock::NewClock(const std::string& str, double FPS = 60.0f)
	{
		try
		{
			if (clockMap.contains(str))
				throw ClockNameExistException();
			if (FPS<0 || FPS>MAX_FRAME_RATE_PER_SECOND)
				throw ClockOutOfRangeException();
			auto pair = getCycleAndFreqIns();
			auto clkelem = std::make_shared<ClockElem>
				(str, pair.first, 1.0f / FPS * pair.second, 1.0f, false);
			clockMap[str] = clkelem;
		}
		catch (ClockException& e)
		{
			std::cout << "\n::Clock::NewClock()" << e.what() << std::endl;
			return false;
		}
		return true;
	}

	inline bool Clock::EraseClock(const std::string& str)
	{
		try
		{
			auto c = clockMap.begin();
			for (c; c != clockMap.end(); c++)
			{
				if (c->first == str)
				{
					clockMap.erase(c);
					return true;
				}
			}
			if (c == clockMap.end())
				throw ClockNotFoundException();
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::EraseClock()" << exp.what() << std::endl;
		}
		return false;
	}

	inline bool Clock::CopyClock(const std::string& str, const std::string& newclk)
	{
		auto e = this->getIterator(str);
		try
		{
			if (clockMap.contains(newclk))
				throw ClockNameExistException();
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			auto clkelem = std::make_shared<ClockElem>(*e, newclk, getCycleAndFreqIns().first);
			clockMap[newclk] = clkelem;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::CopyClock()" << exp.what() << std::endl;
		}
		return false;
	}

	inline bool Clock::GetUpdate(const std::string& str = "Global")
	{
		return this->isUpdate(str);
	}

	inline bool Clock::GetPause(const std::string& str)
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetFramePerSecond()" << exp.what() << std::endl;
		}
		return e->pause;
	}

	inline double Clock::GetFramePerSecond(const std::string& str = "Global")
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetFramePerSecond()" << exp.what() << std::endl;
		}
		return (this->getFreqNow(e) / e->update_tick);
	}

	inline double Clock::GetElapsed(const std::string& str = "Global")
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			unsigned long long elapsed_cycles = this->getCycleNow(e) - e->ins_cycle;
			unsigned long long freq = getFreqNow(e);
			return (elapsed_cycles * 1000) / freq;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetElapsed()" << exp.what() << std::endl;
		}
		return 0;
	}

	inline double Clock::GetElapsedRelative(const std::string& str = "Global")
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			unsigned long long elapsed_cycles =
				e->relative_tick + (this->getCycleNow(e) - e->cycle) * e->scale;
			unsigned long long freq = getFreqNow(e);
			return (elapsed_cycles * 1000) / freq;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetElapsedRelative()" << exp.what() << std::endl;
		}
		return 0;
	}

	inline double Clock::GetTick(const std::string& str = "Global")
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			unsigned long long elapsed_cycles = e->cycle - e->last_cycle;
			unsigned long long freq = getFreqNow(e);
			return (elapsed_cycles * 1000) / (double)freq;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetTick()" << exp.what() << std::endl;
		}
		return 0.0f;
	}

	inline double Clock::GetTickRelative(const std::string& str = "Global")
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			unsigned long long elapsed_cycles = e->cycle - e->last_cycle;
			unsigned long long freq = getFreqNow(e);
			return (elapsed_cycles * 1000) / (double)freq * e->scale;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetTick()" << exp.what() << std::endl;
		}
		return 0.0f;
	}

	inline void Clock::SetPause(const std::string& str, bool is_pause)
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			if (is_pause == e->pause)
			{
				return;
			}
			else if (e->pause == false)
			{
				std::lock_guard<std::mutex> lock(e->lock);
				e->pause_cycle = this->getCycleNow(e);
				e->pause = true;
				return;
			}
			else
			{
				std::lock_guard<std::mutex> lock(e->lock);
				e->pause = false;
				e->cycle += this->getCycleNow(e) - e->pause_cycle;
				return;
			}
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::SetPause()" << exp.what() << std::endl;
		}
	}

	inline void Clock::SetFramePerSecond(const std::string& str, double i)
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			else if (i<MIN_FRAME_RATE_PER_SECOND || i>MAX_FRAME_RATE_PER_SECOND)
				throw ClockOutOfRangeException();
			std::lock_guard<std::mutex> lock(e->lock);
			e->update_tick = 1.0f / i * this->getFreqNow(e);
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::SetFramePerSecond()" << exp.what() << std::endl;
		}
	}

	inline void Clock::SetFrameScale(const std::string& str, double s)
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			else if (s < MIN_SCALE || s > MAX_SCALE)
				throw ClockOutOfRangeException();
			std::lock_guard<std::mutex> lock(e->lock);
			e->scale = s;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::SetFrameScale()" << exp.what() << std::endl;
		}
	}

	inline void Clock::ResetClockIns(const std::string& str)
	{
		auto e = this->getIterator(str);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			std::lock_guard<std::mutex> lock(e->lock);
			e->ins_cycle = this->getCycleNow(e);
			e->relative_tick = 0;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::SetFrameScale()" << exp.what() << std::endl;
		}
	}

	inline void Clock::DEBUG(const std::string& str = "Global")
	{
		auto e = this->getIterator(str);
		//auto pair = getCycleAndFreqIns();
		//std::cout << pair.first << " " << pair.second << std::endl;
		std::cout << "cycle:" << (this->getCycleNow(e) - e->ins_cycle) * 1000 << std::endl;
		std::cout << "freq:" << getFreqNow(e) << std::endl;

		unsigned long long elapsed_cycles = this->getCycleNow(e) - e->last_cycle;
		unsigned long long freq = getFreqNow(e);
		std::cout << "tick" << (elapsed_cycles * 1000) / (double)freq << "\n\n";
	}

	Clock::Clock()
	{
		std::string str("Global");
		auto pair = getCycleAndFreqIns();
		std::shared_ptr<ClockElem> clk = std::make_shared<ClockElem>
			(str, pair.first, 1.0f / 60.0f * pair.second, 1.0f, false);
		clockMap[str] = clk;
	}

	inline std::shared_ptr<ClockElem> Clock::getIterator(const std::string& str)
	{
		auto search = clockMap.find(str);
		if (search != clockMap.end())
		{
			return search->second;
		}
		return nullptr;
	}

	inline std::pair<ull, ull> Clock::getCycleAndFreqIns()
	{
		std::lock_guard<std::mutex> lock(lock_clk);
		QueryPerformanceCounter(&temp_lint_clk);
		temp_ull_clk = temp_lint_clk.QuadPart;
		QueryPerformanceFrequency(&temp_lint_clk);
		return std::pair<ull, ull>(temp_ull_clk, temp_lint_clk.QuadPart);
	}

	inline ull& Clock::getCycleNow(const std::string& str)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(str);
			if (i.get() == nullptr)
				throw ClockNotFoundException();
			std::lock_guard<std::mutex> lock(i->templock);
			QueryPerformanceCounter(&i->temp_lint);
			i->temp_ull = i->temp_lint.QuadPart;
			return i->temp_ull;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::getCycleNow()" << exp.what() << std::endl;
		}
		ull void_ull = 0;
		return void_ull;
	}

	inline ull& Clock::getCycleNow(std::shared_ptr<ClockElem> i)
	{
		try
		{
			if (i.get() == nullptr)
				throw ClockNotFoundException();
			std::lock_guard<std::mutex> lock(i->templock);
			QueryPerformanceCounter(&i->temp_lint);
			i->temp_ull = i->temp_lint.QuadPart;
			return i->temp_ull;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::getCycleNow()" << exp.what() << std::endl;
		}
		ull void_ull = 0;
		return void_ull;
	}

	inline ull& Clock::getFreqNow(const std::string& str)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(str);
			if (i.get() == nullptr)
				throw ClockNotFoundException();

			std::lock_guard<std::mutex> lock(i->templock);
			QueryPerformanceFrequency(&i->temp_lint);
			i->temp_ull = i->temp_lint.QuadPart;

			return i->temp_ull;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::getFreqNow()" << exp.what() << std::endl;
		}
		ull void_ull = 0;
		return void_ull;
	}

	inline ull& Clock::getFreqNow(std::shared_ptr<ClockElem> i)
	{
		try
		{
			if (i.get() == nullptr)
				throw ClockNotFoundException();

			std::lock_guard<std::mutex> lock(i->templock);
			QueryPerformanceFrequency(&i->temp_lint);
			i->temp_ull = i->temp_lint.QuadPart;

			return i->temp_ull;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::getFreqNow()" << exp.what() << std::endl;
		}
		ull void_ull = 0;
		return void_ull;
	}

	inline bool Clock::isUpdate(const std::string& str)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(str);
			if (i.get() == nullptr)
				throw ClockNotFoundException();

			if (i->pause == true)
			{
				return false;
			}
			std::lock_guard<std::mutex> lock(i->templock);
			QueryPerformanceCounter(&i->temp_lint);
			if (i->temp_lint.QuadPart - i->cycle > i->update_tick)
			{
				i->last_cycle = i->cycle;
				i->cycle = i->temp_lint.QuadPart;
				auto relative_passed = (i->cycle - i->last_cycle) * i->scale;
				i->relative_tick += relative_passed;
#ifdef EVENT_SYSTEM
				EventSystem& eventSystem = EventSystem::Instance();
				QueryPerformanceFrequency(&i->temp_lint);
				auto freq = i->temp_lint.QuadPart;
				for (auto e : i->eventList)
				{
					eventSystem.TriggerEventUpdate(e, relative_passed * 1000 / freq);
				}
#endif // EVENT_SYSTEM
				return true;
			}
			else
			{
				return false;
			}
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::isUpdate()" << exp.what() << std::endl;
		}
		return false;
	}

	inline bool Clock::isUpdate(std::shared_ptr<ClockElem> i)
	{
		try
		{
			if (i.get() == nullptr)
				throw ClockNotFoundException();

			if (i->pause == true)
			{
				return false;
			}
			std::lock_guard<std::mutex> lock(i->templock);
			QueryPerformanceCounter(&i->temp_lint);
			if (i->temp_lint.QuadPart - i->cycle > i->update_tick)
			{
				i->last_cycle = i->cycle;
				i->cycle = i->temp_lint.QuadPart;
				auto relative_passed = (i->cycle - i->last_cycle) * i->scale;
				i->relative_tick += relative_passed;
#ifdef EVENT_SYSTEM
				EventSystem& eventSystem = EventSystem::Instance();
				QueryPerformanceFrequency(&i->temp_lint);
				auto freq = i->temp_lint.QuadPart;
				for (auto e : i->eventList)
				{
					eventSystem.TriggerEventUpdate(e, relative_passed * 1000 / freq);
				}
#endif // EVENT_SYSTEM
				return true;
			}
			else
			{
				return false;
			}
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::isUpdate()" << exp.what() << std::endl;
		}
		return false;
	}

#ifdef EVENT_SYSTEM
	inline void Clock::AddEvent(const std::string& str, const std::string& event)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(str);
			if (i.get() == nullptr)
				throw ClockNotFoundException();
			EventSystem& eventsystem = EventSystem::Instance();
			if (eventsystem.IsEventExistNoException(event) == false)
				throw ClockEventNotFoundException();
			std::lock_guard<std::mutex> lock(i->lock);
			i->eventList.push_back(event);
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::AddEvent()" << exp.what() << std::endl;
		}
	}

	inline void Clock::RemoveEvent(const std::string& str, const std::string& event)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(str);
			if (i.get() == nullptr)
				throw ClockNotFoundException();
			EventSystem& eventsystem = EventSystem::Instance();
			bool isExist = false;
			std::lock_guard<std::mutex> lock(i->lock);
			for (auto e = i->eventList.begin(); e != i->eventList.end(); ++e)
			{
				if (*e == event)
				{
					isExist = true;
					i->eventList.erase(e);
					break;
				}
			}
			if (isExist == false)
				throw ClockEventNotFoundException();
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::RemoveEvent()" << exp.what() << std::endl;
		}
	}

	inline std::vector<std::string>& Clock::GetEventList(const std::string& str)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			if (i.get() == nullptr)
				throw ClockNotFoundException();
			return i->eventList;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetEventList()" << exp.what() << std::endl;
		}
	}

	inline void Clock::ClearEventList(const std::string&)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			if (i.get() == nullptr)
				throw ClockNotFoundException();
			i->eventList.clear();
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetEventList()" << exp.what() << std::endl;
		}
	}
#endif // EVENT_SYSTEM
}