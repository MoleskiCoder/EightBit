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
			auto elapsedTime = m_finishTime - m_startTime;
			auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsedTime).count();

			std::cout << std::endl;
			std::cout << "Cycles = " << m_totalCycles << std::endl;
			std::cout << "Seconds = " << seconds << std::endl;

			auto cyclesPerSecond = m_totalCycles / seconds;
			std::cout.imbue(std::locale(""));
			std::cout << cyclesPerSecond << " cycles/second" << std::endl;
		}


		long long getElapsedSeconds() {
			return std::chrono::duration_cast<std::chrono::seconds>(elapsedTime).count();
		}

		long long getCyclesPerSecond() const {

		}

		void runLoop() {
			m_startTime = std::chrono::system_clock::now();
			m_totalCycles = 0UL;

			auto& cpu = m_board.getCPUMutable();
			while (!cpu.isHalted()) {
				m_totalCycles += cpu.step();
			}

			m_finishTime = std::chrono::system_clock::now();
		}

		void initialise() {
			m_board.initialise();
		}


	private:
		const ConfigurationT& m_configuration;
		BoardT m_board;
		long long m_totalCycles;
		std::chrono::system_clock::time_point m_startTime;
		std::chrono::system_clock::time_point m_finishTime;
	};
}