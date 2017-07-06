#pragma once

#include <chrono>
#include <intrin.h>
#include <iostream>

namespace EightBit {
	template<class ConfigurationT, class BoardT> class TestHarness {
	public:
		TestHarness(const ConfigurationT& configuration)
		: m_board(configuration),
		  m_totalCycles(0),
		  m_instructions(0),
		  m_startHostCycles(0),
		  m_finishHostCycles(0) {
		}

		~TestHarness() {
			std::cout << std::dec << std::endl;
			std::cout.imbue(std::locale(""));

			std::cout << "Guest cycles = " << m_totalCycles << std::endl;
			std::cout << "Seconds = " << getElapsedSeconds() << std::endl;

			std::cout << getCyclesPerSecond() << " MHz" << std::endl;
			std::cout << getInstructionsPerSecond() << " instructions per second" << std::endl;

			auto elapsedHostCycles = m_finishHostCycles - m_startHostCycles;
			std::cout << "Host cycles = " << elapsedHostCycles << std::endl;

			auto efficiency = elapsedHostCycles / m_totalCycles;
			std::cout << "Efficiency = " << efficiency << std::endl;
		}

		std::chrono::steady_clock::duration getElapsedTime() const {
			return m_finishTime - m_startTime;
		}

		double getElapsedSeconds() const {
			return std::chrono::duration_cast<std::chrono::duration<double>>(getElapsedTime()).count();
		}

		double getCyclesPerSecond() const {
			return (m_totalCycles / 1000000 ) / getElapsedSeconds();
		}

		long long getInstructionsPerSecond() {
			auto floating = m_instructions / getElapsedSeconds();
			return (long long)floating;
		}

		void runLoop() {
			m_startTime = now();
			m_totalCycles = m_instructions = 0L;
			m_startHostCycles = currentHostCycles();

			auto& cpu = m_board.CPU();
			while (!cpu.isHalted()) {
				m_totalCycles += cpu.step();
				++m_instructions;
			}

			m_finishHostCycles = currentHostCycles();
			m_finishTime = now();
		}

		void initialise() {
			m_board.initialise();
		}

	private:
		BoardT m_board;
		long long m_totalCycles;
		long long m_instructions;
		std::chrono::steady_clock::time_point m_startTime;
		std::chrono::steady_clock::time_point m_finishTime;
		unsigned __int64 m_startHostCycles;
		unsigned __int64 m_finishHostCycles;

		static std::chrono::steady_clock::time_point now() {
			return std::chrono::steady_clock::now();
		}

		static unsigned __int64 currentHostCycles() {
			return __rdtsc();
		}
	};
}