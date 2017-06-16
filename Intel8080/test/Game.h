#pragma once

#include "Board.h"

class Configuration;

class Game {
public:
	Game(const Configuration& configuration);

	void runLoop();
	void initialise();

private:
	const Configuration& m_configuration;
	mutable Board m_board;
};
