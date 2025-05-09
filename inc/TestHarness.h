#pragma once

#include <cstdint>
#include <chrono>
#include <iostream>

#include "EightBitCompilerDefinitions.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

#ifdef __GNUG__
#include <x86intrin.h>
#endif

namespace EightBit {
	template<class ConfigurationT, class BoardT> class TestHarness final {
	public:
		TestHarness(const ConfigurationT& configuration) noexcept
		: m_board(configuration) {}

		~TestHarness() {
			std::cout << std::dec << std::endl;
			std::cout.imbue(std::locale(""));

			std::cout << "Guest cycles = " << m_totalCycles << std::endl;
			std::cout << "Seconds = " << getElapsedSeconds() << std::endl;

			std::cout << getCyclesPerSecond() / 1'000'000 << " MHz" << std::endl;
			std::cout << getInstructionsPerSecond() << " instructions per second" << std::endl;

			auto elapsedHostCycles = m_finishHostCycles - m_startHostCycles;
			std::cout << "Host cycles = " << elapsedHostCycles << std::endl;

			auto efficiency = elapsedHostCycles / m_totalCycles;
			std::cout << "Efficiency = " << efficiency << std::endl;
		}

		[[nodiscard]] std::chrono::steady_clock::duration getElapsedTime() const {
			return m_finishTime - m_startTime;
		}

		[[nodiscard]] auto getElapsedSeconds() const {
			return std::chrono::duration_cast<std::chrono::duration<double>>(getElapsedTime()).count();
		}

		[[nodiscard]] auto getCyclesPerSecond() const {
			return m_totalCycles / getElapsedSeconds();
		}

		[[nodiscard]] auto getInstructionsPerSecond() {
			auto floating = m_instructions / getElapsedSeconds();
			return (long long)floating;
		}

		void run() noexcept {
			m_startTime = now();
			m_totalCycles = m_instructions = 0L;
			m_startHostCycles = currentHostCycles();

			m_board.initialise();
			m_board.raisePOWER();

			auto& cpu = m_board.CPU();

			while (LIKELY(cpu.powered())) {
				m_totalCycles += cpu.step();
				++m_instructions;
			}

			m_finishHostCycles = currentHostCycles();
			m_finishTime = now();
		}

	private:
		BoardT m_board;
		long long m_totalCycles = 0;
		long long m_instructions = 0;
		std::chrono::steady_clock::time_point m_startTime;
		std::chrono::steady_clock::time_point m_finishTime;
		uint64_t m_startHostCycles = 0;
		uint64_t m_finishHostCycles = 0;

		[[nodiscard]] static auto now() {
			return std::chrono::steady_clock::now();
		}

		[[nodiscard]] static uint64_t currentHostCycles() {
			return __rdtsc();
		}
	};
}
