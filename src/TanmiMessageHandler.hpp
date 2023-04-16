#pragma once
/*****************************************************************//**
 * \file   TanmiEventHandler.hpp
 * \brief  �첽�¼�������
 * 
 * \author tanmika
 * \date   April 2023
 *********************************************************************/
#include "TanmiMessageQuene.hpp"
#include "TanmiListener.hpp"

namespace TanmiEngine
{
	/**
	 * @brief �¼��������
	 */
	class MessageHandler
	{
	public:
		MessageHandler() = default;		//<	Ĭ�Ϲ��캯��
		~MessageHandler() = default;	//<	Ĭ����������
		/**
		* @brief �����¼�
		* 
		* @param id �¼�ID
		* @param cilent ������
		*/
		virtual void Post(EventID id, std::shared_ptr<Listener> cilent)
		{
			listener.Push(cilent);
			listener_id.Push(id);
		}
		/**
		 * @brief ������Ϣ����
		 */
		virtual void Run()
		{
			while (!exit)
			{
				HandleMessage(listener_id.Pop(), std::move(listener.Pop()));
			}
		}
		/**
		* @brief ������Ϣ
		* 
		* @param id �¼�ID
		* @param message ������
		*/
		virtual void HandleMessage(const EventID id, std::shared_ptr<Listener> message)
		{
			message->WakeEvent(id);
		}
		/**
		* @brief �ر���Ϣ������
		*/
		void Exit()
		{
			exit = true;
		}
	protected:
		MessageQueue<std::shared_ptr<Listener>> listener;	///< ����������
		MessageQueue<EventID> listener_id;					///< �¼�ID����
		bool exit = false;									///< �Ƿ��˳�
	};
	/**
	 * @brief �¼�������ࣨ����ʱ�������
	 */
	class MessageHandlerUpdate
	{
	public:
		MessageHandlerUpdate() = default;	//<	Ĭ�Ϲ��캯��
		~MessageHandlerUpdate() = default;	//<	Ĭ����������
		/**
		* @brief �����¼�
		* 
		* @param id �¼�ID
		* @param cilent ������
		* @param ms �¼�������������ʱ�䣨�Ժ���Ϊ��λ��
		*/
		virtual void Post(EventID id, std::shared_ptr<Listener> cilent, double ms)
		{
			listener.Push(cilent);
			listener_id.Push(id);
			listener_t.Push(ms);
		}
		/**
		* @brief ������Ϣ����
		*/
		virtual void Run()
		{
			while (!exit)
			{
				HandleMessage(listener_id.Pop(), std::move(listener.Pop()), listener_t.Pop());
			}
		}
        /* @brief ������Ϣ
		* 
		* @param id �¼�ID
		* @param message ������
		* @param ms �¼�������������ʱ�䣨�Ժ���Ϊ��λ��
		*/
		virtual void HandleMessage(const EventID id, std::shared_ptr<Listener> message, double time)
		{
			message->WakeEventUpdate(id, time);
		}
		/**
		 * @brief �ر���Ϣ������
		 */
		void Exit()
		{
			exit = true;
		}
	protected:
		MessageQueue<std::shared_ptr<Listener>> listener;	//<	����������
		MessageQueue<EventID> listener_id;					//<	�¼�ID����
		MessageQueue<double> listener_t;					//<	�¼�������������ʱ�䣨�Ժ���Ϊ��λ��
		bool exit = false;									//<	�Ƿ��˳�
	};
}