#include "stdafx.h"
#include "Game.h"

#include <algorithm>

Game::Game(const Configuration& configuration)
: m_configuration(configuration),
  m_board(configuration) {
}

void Game::initialise() {
	m_board.initialise();
}

void Game::runLoop() {
	auto& cpu = m_board.getCPUMutable();
	auto cycles = 0;
	while (!cpu.isHalted()) {
		cycles = cpu.step();
	}
}
