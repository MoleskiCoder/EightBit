#include "stdafx.h"
#include "Game.h"

#include <algorithm>

Game::Game(const Configuration& configuration)
: m_configuration(configuration),
  m_board(configuration) {
}

Game::~Game() {

	auto elapsedTime = m_finishTime - m_startTime;
	auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsedTime).count();

	std::cout << "Cycles = " << m_totalCycles << std::endl;
	std::cout << "Seconds = " << seconds << std::endl;

	auto cyclesPerSecond = m_totalCycles / seconds;
	std::cout.imbue(std::locale(""));
	std::cout << cyclesPerSecond << " cycles/second" << std::endl;
}

void Game::initialise() {
	m_board.initialise();
}

void Game::runLoop() {

	m_startTime = std::chrono::system_clock::now();
	m_totalCycles = 0UL;

	auto& cpu = m_board.getCPUMutable();
	while (!cpu.isHalted()) {
		m_totalCycles += cpu.step();
	}

	m_finishTime = std::chrono::system_clock::now();
}
