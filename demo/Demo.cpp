#include"demoEvent.hpp"

using namespace TanmiEngine;
auto main() -> int
{
	srand((unsigned)time(NULL));

	std::string player1 = "Andy";
	std::string player2 = "Bob";
	std::string judge = "Judge";

	Clock& clock = Clock::Instance();
	auto Global = clock.NewClock();
	clock.SetFramePerSecond(Global, 1);

	std::shared_ptr<Player> p1 = std::make_shared<Player>(player1, 1);
	std::shared_ptr<Player> p2 = std::make_shared<Player>(player2, 2);
	std::shared_ptr<Judge> j = std::make_shared<Judge>(judge, 3, 3, Global);

	j->AddPlayer(p1, p2);

	EventSystem& eventSystem = EventSystem::Instance();

	eventSystem.UseMessageHandlerDefault();
	auto gameStart = j->GenStart();
	eventSystem.TriggerEvent(*gameStart);
	std::cout << std::endl;
	while (j->gameOver != true)
	{
		if (clock.GetUpdate(Global))
		{
			std::cout << std::endl;
		}
	}
	return 0;
}