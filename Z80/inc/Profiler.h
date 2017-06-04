#pragma once

#include <array>
#include <cstdint>

class Profiler {
public:
	Profiler();
	~Profiler();

	void addInstruction(uint8_t instruction);
	void addAddress(uint16_t address);

	void dump() const;

private:
	std::array<uint64_t, 0x100> m_instructions;
	std::array<uint64_t, 0x10000> m_addresses;

	void dumpInstructionProfiles() const;
	void dumpAddressProfiles() const;
};

