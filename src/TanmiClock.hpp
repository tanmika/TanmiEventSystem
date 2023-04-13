/**
 * @file	TanmiClock
 * @author	Tanmika
 * @email	tanmika@foxmail.com
 * @date	2023-4-12
 * @brief	基于单例模式的简单时钟系统
 */
#pragma once
#include <iostream>
#include <Windows.h>
#include <time.h>
#include <string>
#include <unordered_map>
#include <mutex>
#include <algorithm>
#include <exception>

#include "TanmiEventSystem.hpp"

namespace TanmiEngine
{
	using ull = unsigned long long;	///< 使用 unsigned long long 定义 ull。
	using lint = LARGE_INTEGER;		///< 使用 LARGE_INTEGER 定义 lint。
	using ClockID = int;			///< 使用 int 定义 ClockID。

	/**
	 * @brief Clock 类异常基类。
	 */
	class ClockException : public std::exception
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection Clock Exception basic";
		}
	};
	/**
	 * @brief 数值越界异常类。
	 */
	class ClockOutOfRangeException : public ClockException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection out_of_range";
		}
	};
	/**
	 * @brief 时钟未找到异常类。
	 */
	class ClockNotFoundException : public ClockException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection clock_not_found";
		}
	};
#ifdef EVENT_SYSTEM
	/**
	 * @brief 时钟事件未找到异常类。
	 */
	class ClockEventNotFoundException :public ClockException
	{
	public:
		virtual const char* what () const throw ()
		{
			return "::Expection event_not_found";
		}
	};
#endif // EVENT_SYSTEM

	/**
	 * @brief 该类用于表示时钟元素，包含一些时钟相关的属性和方法。
	 *
	 * 该类作为时钟元，不可直接被调用
	 */
	class ClockElem
	{
	public:
		~ClockElem() = default;	///< 析构函数
		ClockElem() = delete;	///< 禁用默认构造函数

		 /**
		 * @brief 构造函数，用于复制时钟元素
		 * @param ref 参考元素
		 * @param _id 新时钟元素ID
		 * @param _cycle 新时钟元素的计数值
		 */
		ClockElem(ClockElem& ref, ClockID _id, ull _cycle);

		/**
		* @brief 构造函数，用于创建新时钟元素
		* @param[in] _id 时钟元素ID
		* @param[in] _cycle 计数周期
		* @param[in] _update 更新周期
		* @param[in] _scale 时钟放缩比例
		* @param[in] _pause 是否暂停
		*/
		ClockElem(ClockID _id, ull _cycle, ull _update, float _scale, bool _pause) :
			id(_id), cycle(_cycle), last_cycle(_cycle), update_tick(_update), ins_cycle(_cycle),
			pause_cycle(0), relative_tick(0), scale(_scale), pause(_pause), temp_ull(0), temp_lint({})
		{}

		//
		// cycle -> 绝对周期, tick -> 相对周期
		//
		ClockID id;				///< 时钟元素ID
		ull cycle;              ///< 上次唤起更新计数点
		ull last_cycle;         ///< 上次有效更新帧计数点
		ull update_tick;        ///< 更新周期
		ull ins_cycle;          ///< 初始时计数点
		ull pause_cycle;        ///< 暂停时刻计数点
		ull relative_tick;      ///< 相对经过的总计数
		float scale;            ///< 放缩率
		bool pause;             ///< 是否暂停
		ull temp_ull;           ///< 缓冲
		lint temp_lint;         ///< 缓冲
		std::mutex lock;        ///< 数据锁
		std::mutex templock;    ///< 缓冲锁

#ifdef EVENT_SYSTEM
		std::vector<std::shared_ptr<Event>> eventList;  ///< 事件列表
#endif // EVENT_SYSTEM
	};

	ClockElem::ClockElem(ClockElem& e, ClockID _id, ull _cycle) : id(_id), cycle(_cycle), last_cycle(_cycle), ins_cycle(_cycle)
	{
		this->update_tick = e.update_tick;
		this->pause_cycle = e.pause_cycle;
		this->relative_tick = e.relative_tick;
		this->scale = e.scale;
		this->pause = e.pause;
		this->temp_ull = e.temp_ull;
		this->temp_lint = e.temp_lint;
	}

	/**
	 * @brief 该类定义了一个基于帧计数的计时器系统
	 *
	 * 基于单例模式实现
	 */
	class Clock
	{
	private:
		/**
		 * @brief 一个事件名到 ClockElem 共享指针的哈希表，用于存储计时器元素
		 */
		std::unordered_map<ClockID, std::shared_ptr<ClockElem>> clockMap;
		std::mutex lock_clk;	///< 互斥锁，用于保证线程安全
		ull temp_ull_clk;		///< 类内缓存ull时钟计数
		lint temp_lint_clk;		///< 类内缓存lint时钟计数
	private:
		const int MAX_FRAME_RATE_PER_SECOND = 1000;		///< 最大帧率
		const double MIN_FRAME_RATE_PER_SECOND = 0.001;	///< 最小帧率
		const int MAX_SCALE = 1000;			///< 最大缩放倍数
		const double MIN_SCALE = 0.001;		///< 最小缩放倍数
	private:
		Clock();	///< 构造函数 
		Clock(Clock&) = delete;	///< 禁用拷贝构造函数
		Clock& operator=(Clock&) = delete;	///< 禁用拷贝赋值运算符
		Clock(Clock&&) = delete;	///< 禁用移动构造函数
		Clock& operator=(Clock&&) = delete;	///< 禁用移动赋值运算符
		//----------tool----------
		/**
		* @brief 获取新的时钟ID
		* @return 新的时钟ID
		*/
		ClockID getNewID();	
		/**
		* @brief 获取时钟对象指针，失败时返回nullptr
		* @param _id 时钟对象ID
		* @return std::shared_ptr<ClockElem> 时钟对象指针，若找不到则返回nullptr
		*/
		inline std::shared_ptr<ClockElem> getIterator(const ClockID _id);
		/**
		* @brief 获取ull时钟计数与频率，使用类内缓冲
		*
		* @return std::pair<ull, ull> ull时钟计数和频率
		*/
		inline std::pair<ull, ull> getCycleAndFreqIns();
		/**
		* @brief 获取ull时钟计数，使用元素缓冲
		*
		* @param _id 时钟对象ID
		* @return ull& 时钟计数
		*/
		inline ull& getCycleNow(const ClockID _ID);
		/**
		* @brief 获取ull时钟计数，使用元素缓冲
		*
		* @param i 时钟对象指针
		* @return ull& 时钟计数
		*/
		inline ull& getCycleNow(std::shared_ptr<ClockElem> i);
		/**
		* @brief 获取ull时钟频率，使用元素缓冲
		*
		* @param _id 时钟对象ID
		* @return ull& 时钟频率
		*/
		inline ull& getFreqNow(const ClockID _id);
		/**
		* @brief 获取ull时钟频率，使用元素缓冲
		*
		* @param i 时钟对象指针
		* @return ull& 时钟频率
		*/
		inline ull& getFreqNow(std::shared_ptr<ClockElem> i);
		//----------function----------
		/**
		 * @brief 获取时钟是否超过更新点，是则更新时钟
		 * @param _id 时钟ID
		 * @return 是否需要更新时钟
		 */
		inline bool isUpdate(const ClockID _id);
		/**
		 * @brief 获取时钟是否超过更新点，是则更新时钟
		 * @param i 时钟元素指针
		 * @return 是否需要更新时钟
		 */
		inline bool isUpdate(std::shared_ptr<ClockElem> i);

	public:
		/**
		 * @brief 获取Clock实例引用
		 * @return Clock& Clock实例引用
		 */
		static Clock& Instance();
		//----------elemFunction----------
		/**
		 * @brief 新建时钟
		 * @param _id 时钟ID
		 * @param _fps 刷新率
		 * @return 
		 */
		ClockID NewClock(double _fps);
		/**
		 * @brief 移除时钟
		 * @param _id 时钟ID
		 * @return true 移除成功
		 * @return false 移除失败
		 */
		bool EraseClock(const ClockID _id);
		/**
		 * @brief 复制时钟
		 * @param _id 被复制的时钟ID
		 * @return 新时钟ID
		 */
		ClockID CopyClock(const ClockID _id);
		//----------getFunction----------
		/**
		 * @brief 获取时钟是否超过更新点，是则更新时钟
		 * @param _id 时钟ID
		 * @return true 时钟需要更新
		 * @return false 时钟不需要更新
		 */
		bool GetUpdate(const ClockID _id);
		/**
		 * @brief 获取时钟是否暂停
		 * @param _id 时钟ID
		 * @return true 时钟暂停
		 * @return false 时钟不暂停
		 */
		bool GetPause(const ClockID _id);
		/**
		 * @brief 获取时钟刷新率
		 * @param _id 时钟ID
		 * @return double 刷新率
		 */
		double GetFramePerSecond(const ClockID _id);
		/**
		 * @brief 获取时钟自创建以来经过的绝对时长
		 * @param _id 时钟ID
		 * @return double 绝对时长
		 */
		double GetElapsed(const ClockID _id);
		/**
		 * @brief 获取时钟自上一刷新节点经过的绝对时长
		 * @param _id 时钟ID
		 * @return double 绝对时长
		 */
		double GetTick(const ClockID _id);
		/**
		 * @brief 获取时钟自创建以来经过的相对时长
		 * @param _id 时钟ID
		 * @return double 相对时长
		 */
		double GetElapsedRelative(const ClockID _id);
		/**
		 * @brief 获取时钟自上一刷新节点经过的相对时长
		 * @param _id 时钟ID
		 * @return double 相对时长
		 */
		double GetTickRelative(const ClockID _id);
		/**
		 * @brief 设置时钟暂停状态
		 * @param _id 时钟ID
		 * @param _pause true表示暂停，false表示继续
		 */
		void SetPause(const ClockID _id, bool _pause);
		/**
		 * @brief 设置时钟刷新率
		 * @param _id 时钟ID
		 * @param _fps 刷新率
		 */
		void SetFramePerSecond(const ClockID _id, double _fps);
		/**
		 * @brief 设置时钟的时间缩放
		 * @param _id 时钟ID
		 * @param _scale 缩放值
		 */
		void SetFrameScale(const ClockID _id, double _scale);
		/**
		 * @brief 重置时钟的计时器
		 * @param _id 时钟ID
		 */
		void ResetClockIns(const ClockID _id);
		/**
		 * @brief 计数器转换（用于调试），将 unsigned long long 类型的计数器转换为毫秒计数器
		 * @param time_ull unsigned long long 类型的计数器
		 * @return 转换后的毫秒计数器
		 */
		inline int ull2ms_freq(ull time_ull)
		{
			auto freq = getCycleAndFreqIns().second;
			return (time_ull * 1000 / freq);
		}

#ifdef EVENT_SYSTEM
		/**
		 * @brief 将事件添加至指定时钟
		 * @param _id 时钟ID
		 * @param event 事件ID
		 */
		void AddEvent(const ClockID _id, const Event& event);
		/**
		 * @brief 移除指定时钟中的事件
		 * @param _id 时钟ID
		 * @param event 事件ID
		 */
		void RemoveEvent(const ClockID _id, const Event& event);
		/**
		 * @brief 获取指定时钟中的事件列表
		 * @param _id 时钟ID
		 * @return 事件列表
		 */
		std::vector<std::shared_ptr<Event>>& GetEventList(const ClockID _id);
		/**
		 * @brief 清空指定时钟中的事件列表
		 * @param _id 时钟ID
		 */
		void ClearEventList(const ClockID _id);
#endif // EVENT_SYSTEM
		/**
		 * @brief 输出调试信息
		 * @param _id 时钟ID
		 */
		void DEBUG(const ClockID _id);
		~Clock() = default;	///<析构函数
	};

	Clock& Clock::Instance()
	{
		static Clock clk;
		return clk;
	}

	inline ClockID Clock::NewClock(double _fps = 60.0f)
	{
		ClockID _id = 0;
		try
		{	
			if (_fps<0 || _fps>MAX_FRAME_RATE_PER_SECOND)
				throw ClockOutOfRangeException();
			_id = getNewID();
			auto pair = getCycleAndFreqIns();
			auto clkelem = std::make_shared<ClockElem>
				(_id, pair.first, 1.0f / _fps * pair.second, 1.0f, false);
			clockMap[_id] = clkelem;
		}
		catch (ClockException& e)
		{
			std::cout << "\n::Clock::NewClock()" << e.what() << std::endl;
			return 0;
		}
		return _id;
	}

	inline bool Clock::EraseClock(const ClockID _id)
	{
		try
		{
			auto c = clockMap.begin();
			for (c; c != clockMap.end(); c++)
			{
				if (c->first == _id)
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

	inline ClockID Clock::CopyClock(const ClockID _id)
	{
		auto e = this->getIterator(_id);
		ClockID new_id = 0;
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			new_id = getNewID();
			auto clkelem = std::make_shared<ClockElem>(*e, new_id, getCycleAndFreqIns().first);
			clockMap[new_id] = clkelem;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::CopyClock()" << exp.what() << std::endl;
			return 0;
		}
		return new_id;
	}

	inline bool Clock::GetUpdate(const ClockID _id)
	{
		return this->isUpdate(_id);
	}

	inline bool Clock::GetPause(const ClockID _id)
	{
		auto e = this->getIterator(_id);
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

	inline double Clock::GetFramePerSecond(const ClockID _id)
	{
		auto e = this->getIterator(_id);
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

	inline double Clock::GetElapsed(const ClockID _id)
	{
		auto e = this->getIterator(_id);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			unsigned long long elapsed_cycles = this->getCycleNow(e) - e->ins_cycle;
			unsigned long long freq = getFreqNow(e);
			return (elapsed_cycles * 1000.0f) / freq;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetElapsed()" << exp.what() << std::endl;
		}
		return 0;
	}

	inline double Clock::GetElapsedRelative(const ClockID _id)
	{
		auto e = this->getIterator(_id);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			unsigned long long elapsed_cycles =
				e->relative_tick + (this->getCycleNow(e) - e->cycle) * (double)e->scale;
			unsigned long long freq = getFreqNow(e);
			return (elapsed_cycles * 1000.0f) / freq;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetElapsedRelative()" << exp.what() << std::endl;
		}
		return 0;
	}

	inline double Clock::GetTick(const ClockID _id = 1)
	{
		auto e = this->getIterator(_id);
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

	inline double Clock::GetTickRelative(const ClockID _id = 1)
	{
		auto e = this->getIterator(_id);
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

	inline void Clock::SetPause(const ClockID _id, bool _pause)
	{
		auto e = this->getIterator(_id);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			if (_pause == e->pause)
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

	inline void Clock::SetFramePerSecond(const ClockID _id, double _fps)
	{
		auto e = this->getIterator(_id);
		try
		{
			if (e.get() == nullptr)
				throw ClockNotFoundException();
			else if (_fps<MIN_FRAME_RATE_PER_SECOND || _fps>MAX_FRAME_RATE_PER_SECOND)
				throw ClockOutOfRangeException();
			std::lock_guard<std::mutex> lock(e->lock);
			e->update_tick = 1.0f / _fps * this->getFreqNow(e);
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::SetFramePerSecond()" << exp.what() << std::endl;
		}
	}

	inline void Clock::SetFrameScale(const ClockID _id, double s)
	{
		auto e = this->getIterator(_id);
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

	inline void Clock::ResetClockIns(const ClockID _id)
	{
		auto e = this->getIterator(_id);
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

	inline void Clock::DEBUG(const ClockID _id = 1)
	{
		auto e = this->getIterator(_id);
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
		ClockID _id = getNewID();
		auto pair = getCycleAndFreqIns();
		std::shared_ptr<ClockElem> clk = std::make_shared<ClockElem>
			(_id, pair.first, 1.0f / 60.0f * pair.second, 1.0f, false);
		clockMap[_id] = clk;
	}

	inline ClockID Clock::getNewID()
	{
		static int clkID = 1;
		return clkID++;
	}

	inline std::shared_ptr<ClockElem> Clock::getIterator(const ClockID _id)
	{
		auto search = clockMap.find(_id);
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

	inline ull& Clock::getCycleNow(const ClockID _id)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(_id);
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

	inline ull& Clock::getFreqNow(const ClockID _id)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(_id);
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

	inline bool Clock::isUpdate(const ClockID _id)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(_id);
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
				for (auto& e : i->eventList)
				{
					eventSystem.TriggerEventUpdate(*e, relative_passed * 1000 / freq);
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
				for (auto& e : i->eventList)
				{
					eventSystem.TriggerEventUpdate(*e, relative_passed * 1000 / freq);
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
	inline void Clock::AddEvent(const ClockID _id, const Event& event)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(_id);
			if (i.get() == nullptr)
				throw ClockNotFoundException();
			EventSystem& eventsystem = EventSystem::Instance();
			if (eventsystem.IsEventExistNoException(event) == false)
				throw ClockEventNotFoundException();
			std::lock_guard<std::mutex> lock(i->lock);
			auto e = std::make_shared<Event>(event);
			i->eventList.push_back(e);
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::AddEvent()" << exp.what() << std::endl;
		}
	}

	inline void Clock::RemoveEvent(const ClockID _id, const Event& event)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(_id);
			if (i.get() == nullptr)
				throw ClockNotFoundException();
			EventSystem& eventsystem = EventSystem::Instance();
			bool isExist = false;
			std::lock_guard<std::mutex> lock(i->lock);
			auto _e = std::make_shared<Event>(event);
			for (auto e = i->eventList.begin(); e != i->eventList.end(); ++e)
			{
				if (*e == _e)
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

	inline std::vector<std::shared_ptr<Event>>& Clock::GetEventList(const ClockID _id)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(_id);
			if (i.get() == nullptr)
				throw ClockNotFoundException();
			return i->eventList;
		}
		catch (ClockException& exp)
		{
			std::cout << "\n::Clock::GetEventList()" << exp.what() << std::endl;
		}
	}

	inline void Clock::ClearEventList(const ClockID _id)
	{
		std::shared_ptr<ClockElem> i(nullptr);
		try
		{
			i = getIterator(_id);
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