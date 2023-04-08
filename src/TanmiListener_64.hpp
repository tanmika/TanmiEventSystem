//
//	Tanmika --2023.4.8
//
#pragma once
#include <string>

namespace TanmiEngine {
	// �������ӿ�
	class Listener
	{
	public:
		Listener() = default;
		~Listener() = default;
		// �¼�����
		// event:�¼�����
		virtual void WakeEvent(const std::string& event) = 0;
		// �¼�����
		// event:�¼�����
		// ms:����ʱ����
		virtual void WakeEventUpdate(const std::string& event, double ms) = 0;
	};
}