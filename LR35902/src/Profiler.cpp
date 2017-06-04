#include "stdafx.h"
#include "Profiler.h"
#include "LR35902.h"

Profiler::Profiler(LR35902& cpu)
: m_cpu(cpu) {
	std::fill(m_instructions.begin(), m_instructions.end(), 0);
	std::fill(m_addresses.begin(), m_addresses.end(), 0);
}

void Profiler::add(uint16_t address, uint8_t instruction) {

	m_instructions[instruction]++;

	auto old = m_addresses[address];
	if (old == 0)
		std::cout << Disassembler::hex(address) << "\t" << m_disassembler.disassemble(m_cpu) << "\n";

	m_addresses[address]++;
}

void Profiler::dump() const {
	dumpInstructionProfiles();
	dumpAddressProfiles();
}

void Profiler::dumpInstructionProfiles() const {
	std::cout << "** instructions" << std::endl;
	for (int i = 0; i < 0x100; ++i) {
		auto count = m_instructions[i];
		if (count > 0)
			std::cout << Disassembler::hex((uint8_t)i) << "\t" << count << std::endl;
	}
}

void Profiler::dumpAddressProfiles() const {
	std::cout << "** addresses" << std::endl;
	for (int i = 0; i < 0x10000; ++i) {
		auto count = m_addresses[i];
		if (count > 0)
			std::cout << Disassembler::hex((uint16_t)i) << "\t" << count << std::endl;
	}
}
