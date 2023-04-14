//
//	Tanmika --2023.4.2
//
#pragma once

#include <memory>
#include "../src/TanmiEventSystem.hpp"
#include "../src/TanmiClock.hpp"
using namespace TanmiEngine;

class DemoEvent :public Listener
{
public:
	DemoEvent (std::string& _name, int _ID) :name(_name), ID(_ID)
	{};
	~DemoEvent () = default;
	virtual void WakeEvent(const EventID event) = 0;
	virtual void WakeEventUpdate(const EventID event, double ms) = 0;

	void PushEvent(std::shared_ptr<Event>& event)
	{
		eventList.push_back(event);
	};
	std::string name;
protected:
	int ID;
	std::vector<std::shared_ptr<Event>> eventList;
};

enum class Gesture
{
	ROCK,
	PAPER,
	SCISSORS
};

class Player :public DemoEvent
{
public:
	Player(std::string& _name, int _ID) :DemoEvent(_name, ID), winCount(0)
	{};
	~Player() = default;

	virtual void WakeEvent(const EventID event);
	virtual void WakeEventUpdate(const EventID event, double ms);

	void win();
	int getWinCount()const;
	Gesture GetGesture()const;
private:
	int winCount;
	Gesture gesture;
};

inline void Player::WakeEvent(const EventID event)
{}

inline void Player::WakeEventUpdate(const EventID event, double ms)
{
	if (event == eventList[0]->ID)
	{
		int random_number = rand() % 3;
		switch (random_number)
		{
		case 0:
			gesture = Gesture::ROCK;
			break;
		case 1:
			gesture = Gesture::PAPER;
			break;
		case 2:
			gesture = Gesture::SCISSORS;
			break;
		}
		std::cout << name << ": I chooses ";
		switch (gesture)
		{
		case Gesture::ROCK:
			std::cout << "rock!";
			break;
		case Gesture::PAPER:
			std::cout << "paper!";
			break;
		case Gesture::SCISSORS:
			std::cout << "scissors!";
			break;
		}
		std::cout << std::endl;
	}
}

inline void Player::win()
{
	++winCount;
}

inline int Player::getWinCount()const
{
	return winCount;
}

inline Gesture Player::GetGesture() const
{
	return gesture;
}

class Judge :public DemoEvent, public std::enable_shared_from_this<Judge>
{
public:
	Judge(std::string& _name, int _ID, int _winCount, ClockID _clk)
		: DemoEvent(_name, ID), totalCount(1), winCount(_winCount), clk(_clk)
	{};
	~Judge() = default;
	virtual void WakeEvent(const EventID event);
	virtual void WakeEventUpdate(const EventID event, double ms);

	void AddPlayer(std::shared_ptr<Player> _player1, std::shared_ptr<Player> _player2);
	std::shared_ptr<Event> GenStart();
	void GameStart();
	void GameOver();
	bool gameOver = false;
private:
	ClockID clk;
	int totalCount;
	const int winCount;
	std::shared_ptr<Player> player1;
	std::shared_ptr<Player> player2;
};
inline void Judge::WakeEvent(const EventID event)
{
	if (event == eventList[0]->ID)
	{
		GameStart();
	}
}
inline void Judge::WakeEventUpdate(const EventID event, double ms)
{
	if (event == eventList[1]->ID)
	{
		if (player1->GetGesture() == Gesture::ROCK)
		{
			if (player2->GetGesture() == Gesture::ROCK)
			{
				std::cout << name << ": Tie!\n";
			}
			else if (player2->GetGesture() == Gesture::PAPER)
			{
				std::cout << name << ": " << player2->name << " win!\n";
				player2->win();
			}
			else if (player2->GetGesture() == Gesture::SCISSORS)
			{
				std::cout << name << ": " << player1->name << " win!\n";
				player1->win();
			}
		}
		else if (player1->GetGesture() == Gesture::PAPER)
		{
			if (player2->GetGesture() == Gesture::ROCK)
			{
				std::cout << name << ": " << player1->name << " win!\n";
				player1->win();
			}
			else if (player2->GetGesture() == Gesture::PAPER)
			{
				std::cout << name << ": Tie!\n";
			}
			else if (player2->GetGesture() == Gesture::SCISSORS)
			{
				std::cout << name << ": " << player2->name << " win!\n";
				player2->win();
			}
		}
		else if (player1->GetGesture() == Gesture::SCISSORS)
		{
			if (player2->GetGesture() == Gesture::ROCK)
			{
				std::cout << name << ": " << player2->name << " win!\n";
				player2->win();
			}
			else if (player2->GetGesture() == Gesture::PAPER)
			{
				std::cout << name << ": " << player1->name << " win!\n";
				player1->win();
			}
			else if (player2->GetGesture() == Gesture::SCISSORS)
			{
				std::cout << name << ": Tie!\n";
			}
		}
	}
	else if (event == eventList[2]->ID)
	{
		std::cout << name << ": " << player1->name << " win " << player1->getWinCount() << " times, " << player2->name << " win " << player2->getWinCount() << " times.\n";
		if (player1->getWinCount() >= winCount)
		{
			std::cout << name << ": Game over! " << player1->name << " win the game!\n";
			GameOver();
		}
		else if (player2->getWinCount() >= winCount)
		{
			std::cout << name << ": Game over! " << player2->name << " win the game!\n";
			GameOver();
		}
	}
	else if (event == eventList[3]->ID)
	{
		std::cout << name << ": Round " << totalCount++ << ".\n";
	}
}

inline void Judge::AddPlayer(std::shared_ptr<Player> _player1, std::shared_ptr<Player> _player2)
{
	player1 = _player1;
	player2 = _player2;
}

inline std::shared_ptr<Event> Judge::GenStart()
{
	EventSystem& eventSystem = EventSystem::Instance();
	auto gameStart = eventSystem.NewAndRegisterEvent<Event>();
	eventSystem.AddEventHandler(*gameStart, shared_from_this());
	eventList.push_back(gameStart);
	return gameStart;
}

inline void Judge::GameStart()
{
	try
	{
		if (player1.get() == nullptr || player2.get() == nullptr)
			throw std::exception("player is not defined");

		TanmiEngine::EventSystem& eventSystem = TanmiEngine::EventSystem::Instance();

		auto game = std::make_shared<Event>();
		eventSystem.RegisterEvent(*game);
		eventSystem.AddEventHandler(*game, player1);
		player1->PushEvent(game);
		eventSystem.AddEventHandler(*game, player2);
		player2->PushEvent(game);
		auto judgement = eventSystem.NewAndRegisterEvent<Event>();
		eventSystem.AddEventHandler(*judgement, shared_from_this());
		PushEvent(judgement);
		auto announce = eventSystem.NewAndRegisterEvent<Event>();
		eventSystem.AddEventHandler(*announce, shared_from_this());
		PushEvent(announce);
		auto begin = eventSystem.NewAndRegisterEvent<Event>();
		eventSystem.AddEventHandler(*begin, shared_from_this());
		PushEvent(begin);

		PushEvent(game);

		TanmiEngine::Clock& clock = TanmiEngine::Clock::Instance();
		clock.AddEvent(clk, *begin);
		clock.AddEvent(clk, *game);
		clock.AddEvent(clk, *judgement);
		clock.AddEvent(clk, *announce);

		std::cout << "Welcome to this exciting Rock-Paper-Scissors game, where player 1, "
			<< player1->name << ", will face off against player 2, " << player2->name
			<< ".\nThe first player to win 3 rounds will be declared the winner of this game!\n";
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}

inline void Judge::GameOver()
{
	TanmiEngine::EventSystem& eventSystem = TanmiEngine::EventSystem::Instance();
	eventSystem.RemoveAllEventForEventName(*eventList[4]);
	eventSystem.RemoveAllEventForEventName(*eventList[1]);
	TanmiEngine::Clock& clock = TanmiEngine::Clock::Instance();
	clock.RemoveEvent(clk, *eventList[4]);
	clock.RemoveEvent(clk, *eventList[1]);
	gameOver = true;
}