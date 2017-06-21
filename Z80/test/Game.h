#pragma once

#include "Board.h"

#include <chrono>

class Configuration;

class Game {
public:
	Game(const Configuration& configuration);
	~Game();

	void runLoop();
	void initialise();

private:
	const Configuration& m_configuration;
	Board m_board;
	long long m_totalCycles;
	std::chrono::system_clock::time_point m_startTime;
	std::chrono::system_clock::time_point m_finishTime;
};
