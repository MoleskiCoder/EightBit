#pragma once

#include <chrono>

namespace EightBit {
	template<class ConfigurationT, class BoardT> class TestHarness {
	public:
		TestHarness(const ConfigurationT& configuration)
		: m_configuration(configuration),
		  m_board(configuration) {
		}

		~TestHarness() {
			std::cout << std::endl;
			std::cout.imbue(std::locale(""));

			std::cout << "Cycles = " << m_totalCycles << std::endl;
			std::cout << "Seconds = " << getElapsedSeconds() << std::endl;

			std::cout << getCyclesPerSecond() << " cycles/second" << std::endl;
		}

		std::chrono::steady_clock::duration getElapsedTime() const {
			return m_finishTime - m_startTime;
		}

		long long getElapsedSeconds() const {
			return std::chrono::duration_cast<std::chrono::seconds>(getElapsedTime()).count();
		}

		long long getCyclesPerSecond() const {
			return m_totalCycles / getElapsedSeconds();
		}

		void runLoop() {
			m_startTime = now();
			m_totalCycles = 0UL;

			auto& cpu = m_board.getCPUMutable();
			while (!cpu.isHalted()) {
				m_totalCycles += cpu.step();
			}

			m_finishTime = now();
		}

		void initialise() {
			m_board.initialise();
		}

	private:
		const ConfigurationT& m_configuration;
		BoardT m_board;
		long long m_totalCycles;
		std::chrono::steady_clock::time_point m_startTime;
		std::chrono::steady_clock::time_point m_finishTime;

		std::chrono::steady_clock::time_point now() {
			return std::chrono::steady_clock::now();
		}
	};
}