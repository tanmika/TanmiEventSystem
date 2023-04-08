#include"demoEvent.hpp"

using namespace TanmiEngine;
auto main() -> int
{
	srand((unsigned)time(NULL));

	std::string player1 = "Andy";
	std::string player2 = "Bob";
	std::string judge = "Judge";

	std::shared_ptr<Player> p1 = std::make_shared<Player>(player1, 1);
	std::shared_ptr<Player> p2 = std::make_shared<Player>(player2, 2);
	std::shared_ptr<Judge> j = std::make_shared<Judge>(judge, 3, 3);

	j->AddPlayer(p1, p2);

	Clock& clock = Clock::Instance();
	clock.SetFramePerSecond("Global", 1);

	EventSystem& eventSystem = EventSystem::Instance();
	eventSystem.AddEventHandler("GameStart", j);
	eventSystem.TriggerEvent("GameStart");
	std::cout << std::endl;
	while (j->gameOver != true)
	{
		if (clock.GetUpdate())
		{
			std::cout << std::endl;
		}
	}
	return 0;
}