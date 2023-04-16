#pragma once
/*****************************************************************//**
 * \file   TanmiEvent.hpp
 * \brief  �¼�����
 * 
 * \author tanmika
 * \date   April 2023
 *********************************************************************/
using EventID = int;
namespace TanmiEngine {
	/**
	 * @brief �¼�����
	 */
	class Event
	{
	public:
		/**
		 * @brief ���캯������ʼ���¼�IDΪ0
		 * 
		 */
		Event() :ID(0)
		{}
		/**
		* @brief Ĭ����������
		*/
		~Event() = default;
		/**
		 * @brief Ԥ������������ʵ��Ԥ�����߼�
		 *
		 * @return true Ԥ����ɹ���ִ���¼�
		 * @return false Ԥ����ʧ�ܣ���ִ���¼�
		 */
		virtual bool preProcess()
		{
			return true;
		}
		EventID ID;	///< �¼�ID
	};
}