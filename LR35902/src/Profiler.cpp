#include "stdafx.h"
#include "../inc/Profiler.h"
#include "../inc/LR35902.h"

EightBit::GameBoy::Profiler::Profiler(Bus& bus, LR35902& cpu)
: m_bus(bus),
  m_cpu(cpu),
  m_disassembler(bus) {
	std::fill(m_instructions.begin(), m_instructions.end(), 0);
	std::fill(m_addresses.begin(), m_addresses.end(), 0);
}

void EightBit::GameBoy::Profiler::add(const uint16_t address, const uint8_t instruction) {

	m_instructions[instruction]++;

	auto old = m_addresses[address];
	if (old == 0)
		std::cout << Disassembler::hex(address) << "\t" << m_disassembler.disassemble(m_cpu) << "\n";

	m_addresses[address]++;
}

void EightBit::GameBoy::Profiler::dump() const {
	dumpInstructionProfiles();
	dumpAddressProfiles();
}

void EightBit::GameBoy::Profiler::dumpInstructionProfiles() const {
	std::cout << "** instructions" << std::endl;
	for (int i = 0; i < 0x100; ++i) {
		auto count = m_instructions[i];
		if (count > 0)
			std::cout << Disassembler::hex((uint8_t)i) << "\t" << count << std::endl;
	}
}

void EightBit::GameBoy::Profiler::dumpAddressProfiles() const {
	std::cout << "** addresses" << std::endl;
	for (int i = 0; i < 0x10000; ++i) {
		auto count = m_addresses[i];
		if (count > 0)
			std::cout << Disassembler::hex((uint16_t)i) << "\t" << count << std::endl;
	}
}
