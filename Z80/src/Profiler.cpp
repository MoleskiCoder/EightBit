#include "stdafx.h"
#include "Profiler.h"
#include "Disassembler.h"

EightBit::Profiler::Profiler() {
	std::fill(m_instructions.begin(), m_instructions.end(), 0);
	std::fill(m_addresses.begin(), m_addresses.end(), 0);
}

EightBit::Profiler::~Profiler() {
}

void EightBit::Profiler::addInstruction(uint8_t instruction) {
	m_instructions[instruction]++;
}

void EightBit::Profiler::addAddress(uint16_t address) {
	m_addresses[address]++;
}

void EightBit::Profiler::dump() const {
	dumpInstructionProfiles();
	dumpAddressProfiles();
}

void EightBit::Profiler::dumpInstructionProfiles() const {
	std::cout << "** instructions" << std::endl;
	for (int i = 0; i < 0x100; ++i) {
		auto count = m_instructions[i];
		if (count > 0)
			std::cout << Disassembler::hex((uint8_t)i) << "\t" << count << std::endl;
	}
}

void EightBit::Profiler::dumpAddressProfiles() const {
	std::cout << "** addresses" << std::endl;
	for (int i = 0; i < 0x10000; ++i) {
		auto count = m_addresses[i];
		if (count > 0)
			std::cout << Disassembler::hex((uint16_t)i) << "\t" << count << std::endl;
	}
}
