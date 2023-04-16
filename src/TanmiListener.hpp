#pragma once
/*****************************************************************//**
 * \file   TanmiListener.hpp
 * \brief  ����������
 * 
 * \author tanmika
 * \date   April 2023
 *********************************************************************/
using EventID = int;
namespace TanmiEngine {
	/**
	 * @brief ����������
	 */
	class Listener
	{
	public:
		/**
		 * @brief Ĭ�Ϲ��캯��
		 */
		Listener() = default;
		/**
		 * @brief Ĭ����������
		 */
		virtual ~Listener() = default;
		/**
		 * @brief �¼���Ӧ����
		 * 
		 * @param event �¼�ID
		 */
		virtual void WakeEvent(const EventID event) = 0;
        /* @brief �¼���Ӧ����
		 * 
		 * @param event �¼�ID
		 * @param ms    �¼�������������ʱ�䣨�Ժ���Ϊ��λ��
		 */
		virtual void WakeEventUpdate(const EventID event, double ms) = 0;
	};
}