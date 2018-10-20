#pragma once

#include <array>
#include <cstdint>

#include "Disassembler.h"
#include "GameBoyBus.h"

namespace EightBit {
	namespace GameBoy {

		class LR35902;

		class Profiler {
		public:
			Profiler(Bus& bus, LR35902& cpu);

			void add(uint16_t address, uint8_t instruction);

			void dump() const;

		private:
			std::array<uint64_t, 0x100> m_instructions;
			std::array<uint64_t, 0x10000> m_addresses;
			Bus& m_bus;
			LR35902& m_cpu;
			Disassembler m_disassembler;

			void dumpInstructionProfiles() const;
			void dumpAddressProfiles() const;
		};
	}
}