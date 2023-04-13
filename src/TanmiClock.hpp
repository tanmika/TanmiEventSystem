/**
 * @file	TanmiClock
 * @author	Tanmika
 * @email	tanmika@foxmail.com
 * @date	2023-4-12
 * @brief	���ڵ���ģʽ�ļ�ʱ��ϵͳ
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
	using ull = unsigned long long;	///< ʹ�� unsigned long long ���� ull��
	using lint = LARGE_INTEGER;		///< ʹ�� LARGE_INTEGER ���� lint��
	using ClockID = int;			///< ʹ�� int ���� ClockID��

	/**
	 * @brief Clock ���쳣���ࡣ
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
	 * @brief ��ֵԽ���쳣�ࡣ
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
	 * @brief ʱ��δ�ҵ��쳣�ࡣ
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
	 * @brief ʱ���¼�δ�ҵ��쳣�ࡣ
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
	 * @brief �������ڱ�ʾʱ��Ԫ�أ�����һЩʱ����ص����Ժͷ�����
	 *
	 * ������Ϊʱ��Ԫ������ֱ�ӱ�����
	 */
	class ClockElem
	{
	public:
		~ClockElem() = default;	///< ��������
		ClockElem() = delete;	///< ����Ĭ�Ϲ��캯��

		 /**
		 * @brief ���캯�������ڸ���ʱ��Ԫ��
		 * @param ref �ο�Ԫ��
		 * @param _id ��ʱ��Ԫ��ID
		 * @param _cycle ��ʱ��Ԫ�صļ���ֵ
		 */
		ClockElem(ClockElem& ref, ClockID _id, ull _cycle);

		/**
		* @brief ���캯�������ڴ�����ʱ��Ԫ��
		* @param[in] _id ʱ��Ԫ��ID
		* @param[in] _cycle ��������
		* @param[in] _update ��������
		* @param[in] _scale ʱ�ӷ�������
		* @param[in] _pause �Ƿ���ͣ
		*/
		ClockElem(ClockID _id, ull _cycle, ull _update, float _scale, bool _pause) :
			id(_id), cycle(_cycle), last_cycle(_cycle), update_tick(_update), ins_cycle(_cycle),
			pause_cycle(0), relative_tick(0), scale(_scale), pause(_pause), temp_ull(0), temp_lint({})
		{}

		//
		// cycle -> ��������, tick -> �������
		//
		ClockID id;				///< ʱ��Ԫ��ID
		ull cycle;              ///< �ϴλ�����¼�����
		ull last_cycle;         ///< �ϴ���Ч����֡������
		ull update_tick;        ///< ��������
		ull ins_cycle;          ///< ��ʼʱ������
		ull pause_cycle;        ///< ��ͣʱ�̼�����
		ull relative_tick;      ///< ��Ծ������ܼ���
		float scale;            ///< ������
		bool pause;             ///< �Ƿ���ͣ
		ull temp_ull;           ///< ����
		lint temp_lint;         ///< ����
		std::mutex lock;        ///< ������
		std::mutex templock;    ///< ������

#ifdef EVENT_SYSTEM
		std::vector<std::shared_ptr<Event>> eventList;  ///< �¼��б�
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
	 * @brief ���ඨ����һ������֡�����ļ�ʱ��ϵͳ
	 *
	 * ���ڵ���ģʽʵ��
	 */
	class Clock
	{
	private:
		/**
		 * @brief һ���¼����� ClockElem ����ָ��Ĺ�ϣ�����ڴ洢��ʱ��Ԫ��
		 */
		std::unordered_map<ClockID, std::shared_ptr<ClockElem>> clockMap;
		std::mutex lock_clk;	///< �����������ڱ�֤�̰߳�ȫ
		ull temp_ull_clk;		///< ���ڻ���ullʱ�Ӽ���
		lint temp_lint_clk;		///< ���ڻ���lintʱ�Ӽ���
	private:
		const int MAX_FRAME_RATE_PER_SECOND = 1000;		///< ���֡��
		const double MIN_FRAME_RATE_PER_SECOND = 0.001;	///< ��С֡��
		const int MAX_SCALE = 1000;			///< ������ű���
		const double MIN_SCALE = 0.001;		///< ��С���ű���
	private:
		Clock();	///< ���캯�� 
		Clock(Clock&) = delete;	///< ���ÿ������캯��
		Clock& operator=(Clock&) = delete;	///< ���ÿ�����ֵ�����
		Clock(Clock&&) = delete;	///< �����ƶ����캯��
		Clock& operator=(Clock&&) = delete;	///< �����ƶ���ֵ�����
		//----------tool----------
		/**
		* @brief ��ȡ�µ�ʱ��ID
		* @return �µ�ʱ��ID
		*/
		ClockID getNewID();	
		/**
		* @brief ��ȡʱ�Ӷ���ָ�룬ʧ��ʱ����nullptr
		* @param _id ʱ�Ӷ���ID
		* @return std::shared_ptr<ClockElem> ʱ�Ӷ���ָ�룬���Ҳ����򷵻�nullptr
		*/
		inline std::shared_ptr<ClockElem> getIterator(const ClockID _id);
		/**
		* @brief ��ȡullʱ�Ӽ�����Ƶ�ʣ�ʹ�����ڻ���
		*
		* @return std::pair<ull, ull> ullʱ�Ӽ�����Ƶ��
		*/
		inline std::pair<ull, ull> getCycleAndFreqIns();
		/**
		* @brief ��ȡullʱ�Ӽ�����ʹ��Ԫ�ػ���
		*
		* @param _id ʱ�Ӷ���ID
		* @return ull& ʱ�Ӽ���
		*/
		inline ull& getCycleNow(const ClockID _ID);
		/**
		* @brief ��ȡullʱ�Ӽ�����ʹ��Ԫ�ػ���
		*
		* @param i ʱ�Ӷ���ָ��
		* @return ull& ʱ�Ӽ���
		*/
		inline ull& getCycleNow(std::shared_ptr<ClockElem> i);
		/**
		* @brief ��ȡullʱ��Ƶ�ʣ�ʹ��Ԫ�ػ���
		*
		* @param _id ʱ�Ӷ���ID
		* @return ull& ʱ��Ƶ��
		*/
		inline ull& getFreqNow(const ClockID _id);
		/**
		* @brief ��ȡullʱ��Ƶ�ʣ�ʹ��Ԫ�ػ���
		*
		* @param i ʱ�Ӷ���ָ��
		* @return ull& ʱ��Ƶ��
		*/
		inline ull& getFreqNow(std::shared_ptr<ClockElem> i);
		//----------function----------
		/**
		 * @brief ��ȡʱ���Ƿ񳬹����µ㣬�������ʱ��
		 * @param _id ʱ��ID
		 * @return �Ƿ���Ҫ����ʱ��
		 */
		inline bool isUpdate(const ClockID _id);
		/**
		 * @brief ��ȡʱ���Ƿ񳬹����µ㣬�������ʱ��
		 * @param i ʱ��Ԫ��ָ��
		 * @return �Ƿ���Ҫ����ʱ��
		 */
		inline bool isUpdate(std::shared_ptr<ClockElem> i);

	public:
		/**
		 * @brief ��ȡClockʵ������
		 * @return Clock& Clockʵ������
		 */
		static Clock& Instance();
		//----------elemFunction----------
		/**
		 * @brief �½�ʱ��
		 * @param _id ʱ��ID
		 * @param _fps ˢ����
		 * @return 
		 */
		ClockID NewClock(double _fps);
		/**
		 * @brief �Ƴ�ʱ��
		 * @param _id ʱ��ID
		 * @return true �Ƴ��ɹ�
		 * @return false �Ƴ�ʧ��
		 */
		bool EraseClock(const ClockID _id);
		/**
		 * @brief ����ʱ��
		 * @param _id �����Ƶ�ʱ��ID
		 * @return ��ʱ��ID
		 */
		ClockID CopyClock(const ClockID _id);
		//----------getFunction----------
		/**
		 * @brief ��ȡʱ���Ƿ񳬹����µ㣬�������ʱ��
		 * @param _id ʱ��ID
		 * @return true ʱ����Ҫ����
		 * @return false ʱ�Ӳ���Ҫ����
		 */
		bool GetUpdate(const ClockID _id);
		/**
		 * @brief ��ȡʱ���Ƿ���ͣ
		 * @param _id ʱ��ID
		 * @return true ʱ����ͣ
		 * @return false ʱ�Ӳ���ͣ
		 */
		bool GetPause(const ClockID _id);
		/**
		 * @brief ��ȡʱ��ˢ����
		 * @param _id ʱ��ID
		 * @return double ˢ����
		 */
		double GetFramePerSecond(const ClockID _id);
		/**
		 * @brief ��ȡʱ���Դ������������ľ���ʱ��
		 * @param _id ʱ��ID
		 * @return double ����ʱ��
		 */
		double GetElapsed(const ClockID _id);
		/**
		 * @brief ��ȡʱ������һˢ�½ڵ㾭���ľ���ʱ��
		 * @param _id ʱ��ID
		 * @return double ����ʱ��
		 */
		double GetTick(const ClockID _id);
		/**
		 * @brief ��ȡʱ���Դ����������������ʱ��
		 * @param _id ʱ��ID
		 * @return double ���ʱ��
		 */
		double GetElapsedRelative(const ClockID _id);
		/**
		 * @brief ��ȡʱ������һˢ�½ڵ㾭�������ʱ��
		 * @param _id ʱ��ID
		 * @return double ���ʱ��
		 */
		double GetTickRelative(const ClockID _id);
		/**
		 * @brief ����ʱ����ͣ״̬
		 * @param _id ʱ��ID
		 * @param _pause true��ʾ��ͣ��false��ʾ����
		 */
		void SetPause(const ClockID _id, bool _pause);
		/**
		 * @brief ����ʱ��ˢ����
		 * @param _id ʱ��ID
		 * @param _fps ˢ����
		 */
		void SetFramePerSecond(const ClockID _id, double _fps);
		/**
		 * @brief ����ʱ�ӵ�ʱ������
		 * @param _id ʱ��ID
		 * @param _scale ����ֵ
		 */
		void SetFrameScale(const ClockID _id, double _scale);
		/**
		 * @brief ����ʱ�ӵļ�ʱ��
		 * @param _id ʱ��ID
		 */
		void ResetClockIns(const ClockID _id);
		/**
		 * @brief ������ת�������ڵ��ԣ����� unsigned long long ���͵ļ�����ת��Ϊ���������
		 * @param time_ull unsigned long long ���͵ļ�����
		 * @return ת����ĺ��������
		 */
		inline int ull2ms_freq(ull time_ull)
		{
			auto freq = getCycleAndFreqIns().second;
			return (time_ull * 1000 / freq);
		}

#ifdef EVENT_SYSTEM
		/**
		 * @brief ���¼������ָ��ʱ��
		 * @param _id ʱ��ID
		 * @param event �¼�ID
		 */
		void AddEvent(const ClockID _id, const Event& event);
		/**
		 * @brief �Ƴ�ָ��ʱ���е��¼�
		 * @param _id ʱ��ID
		 * @param event �¼�ID
		 */
		void RemoveEvent(const ClockID _id, const Event& event);
		/**
		 * @brief ��ȡָ��ʱ���е��¼��б�
		 * @param _id ʱ��ID
		 * @return �¼��б�
		 */
		std::vector<std::shared_ptr<Event>>& GetEventList(const ClockID _id);
		/**
		 * @brief ���ָ��ʱ���е��¼��б�
		 * @param _id ʱ��ID
		 */
		void ClearEventList(const ClockID _id);
#endif // EVENT_SYSTEM
		/**
		 * @brief ���������Ϣ
		 * @param _id ʱ��ID
		 */
		void DEBUG(const ClockID _id);
		~Clock() = default;	///<��������
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