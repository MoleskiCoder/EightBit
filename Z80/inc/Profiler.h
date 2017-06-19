#pragma once

#include <array>
#include <cstdint>

namespace EightBit {

	class Disassembler;
	class Z80;

	class Profiler {
	public:
		Profiler(Z80& cpu, Disassembler& disassembler);
		~Profiler();

		void addInstruction(uint8_t instruction);
		void addAddress(uint16_t address);

		void dump() const;

	private:
		std::array<uint64_t, 0x100> m_instructions;
		std::array<uint64_t, 0x10000> m_addresses;
		Disassembler& m_disassembler;
		Z80& m_cpu;

		void dumpInstructionProfiles() const;
		void dumpAddressProfiles() const;
	};
}