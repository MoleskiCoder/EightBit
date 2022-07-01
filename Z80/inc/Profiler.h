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
		Z80& m_cpu;
		Disassembler& m_disassembler;

		[[nodiscard]] constexpr const auto& instructions() const { return m_instructions; }
		[[nodiscard]] constexpr const auto& addresses() const { return m_addresses; }

		[[nodiscard]] constexpr auto instructions_available() const noexcept {
			const auto found = std::find_if(
				instructions().begin(), instructions().end(),
				[](const auto& value) { return value != 0; });
			return found != instructions().end();
		}

		[[nodiscard]] constexpr auto addresses_available() const noexcept {
			const auto found = std::find_if(
				addresses().begin(), addresses().end(),
				[](const auto& value) { return value != 0; });
			return found != addresses().end();
		}

		[[nodiscard]] constexpr auto available() const noexcept {
			return instructions_available() || addresses_available();
		}

		void dumpInstructionProfiles() const;
		void dumpAddressProfiles() const;
	};
}