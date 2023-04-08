//
//	Tanmika --2023.4.2
//
#pragma once

#include <memory>
#include "../src/TanmiEventSystem_64.hpp"
#include "../src/TanmiClock_64.hpp"
using namespace TanmiEngine;

class DemoEvent :public Listener
{
public:
	DemoEvent (std::string& _name, int _ID) :name(_name), ID(_ID)
	{};
	~DemoEvent () = default;
	virtual void WakeEvent(const std::string& event) = 0;
	virtual void WakeEventUpdate(const std::string& event, double ms) = 0;
	std::string name;
protected:
	int ID;
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

	virtual void WakeEvent(const std::string& event);
	virtual void WakeEventUpdate(const std::string& event, double ms);

	void win();
	int getWinCount()const;
	Gesture GetGesture()const;
private:
	int winCount;
	Gesture gesture;
};

inline void Player::WakeEvent(const std::string& event)
{}

inline void Player::WakeEventUpdate(const std::string& event, double ms)
{
	if (event == "Game")
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
	Judge(std::string& _name, int _ID, int _winCount)
		: DemoEvent(_name, ID), totalCount(1), winCount(_winCount)
	{};
	~Judge() = default;
	virtual void WakeEvent(const std::string& event);
	virtual void WakeEventUpdate(const std::string& event, double ms);

	void AddPlayer(std::shared_ptr<Player> _player1, std::shared_ptr<Player> _player2);
	void GameStart();
	void GameOver();
	bool gameOver = false;
private:
	int totalCount;
	const int winCount;
	std::shared_ptr<Player> player1;
	std::shared_ptr<Player> player2;
};
inline void Judge::WakeEvent(const std::string& event)
{
	if (event == "GameStart")
	{
		GameStart();
	}
}
inline void Judge::WakeEventUpdate(const std::string& event, double ms)
{
	if (event == "Judgement")
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
	else if(event =="Announce")
	{
		std::cout<<name<<": "<<player1->name<<" win "<<player1->getWinCount()<<" times, "<<player2->name<<" win "<<player2->getWinCount()<<" times.\n";
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
	else if (event == "Begin")
	{
		std::cout << name << ": Round " << totalCount++ << ".\n";
	}
}

inline void Judge::AddPlayer(std::shared_ptr<Player> _player1, std::shared_ptr<Player> _player2)
{
	player1 = _player1;
	player2 = _player2;
}

inline void Judge::GameStart()
{
	try
	{
		if (player1.get() == nullptr || player2.get() == nullptr)
			throw std::exception("player is not defined");

		TanmiEngine::EventSystem& eventSystem = TanmiEngine::EventSystem::Instance();
		eventSystem.AddEventHandler("Game", player1);
		eventSystem.AddEventHandler("Game", player2);
		eventSystem.AddEventHandler("Judgement", shared_from_this());
		eventSystem.AddEventHandler("Announce", shared_from_this());
		eventSystem.AddEventHandler("Begin", shared_from_this());

		TanmiEngine::Clock& clock = TanmiEngine::Clock::Instance();
		clock.AddEvent("Global", "Begin");
		clock.AddEvent("Global", "Game");
		clock.AddEvent("Global", "Judgement");
		clock.AddEvent("Global", "Announce");

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
	eventSystem.RemoveAllEventForEventName("Game");
	eventSystem.RemoveAllEventForEventName("Judgement");
	TanmiEngine::Clock& clock = TanmiEngine::Clock::Instance();
	clock.RemoveEvent("Global", "Game");
	clock.RemoveEvent("Global", "Judgement");
	gameOver = true;
}

