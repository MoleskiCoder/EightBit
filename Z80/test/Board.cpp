#include "stdafx.h"
#include "Board.h"

#include <Disassembler.h>

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_ram(0x10000),
  m_cpu(EightBit::Z80(*this, m_ports)),
  m_profiler(m_cpu, m_disassembler) {
}

void Board::initialise() {

	auto romDirectory = m_configuration.getRomDirectory();

	//m_ram.load(romDirectory + "/prelim.com", 0x100);	// Bartholomew preliminary
	//m_ram.load(romDirectory + "/zexdoc.com", 0x100);	// Cringle/Bartholomew
	m_ram.load(romDirectory + "/zexall.com", 0x100);	// Cringle/Bartholomew
	//m_ram.load(romDirectory + "/CPUTEST.COM", 0x100);	// SuperSoft diagnostics
	//m_ram.load(romDirectory + "/TEST.COM", 0x100);		// Microcosm

	poke(5, 0xc9);	// ret
	m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Cpm, this, std::placeholders::_1));

	if (m_configuration.isProfileMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	m_cpu.initialise();
	m_cpu.PC() = m_configuration.getStartAddress();
}

void Board::Cpu_ExecutingInstruction_Cpm(const EightBit::Z80&) {
	auto pc = m_cpu.PC();
	switch (pc.word) {
	case 0x0:	// CP/M warm start
		m_cpu.halt();
		if (m_configuration.isProfileMode()) {
			m_profiler.dump();
		}
		break;
	case 0x5:	// BDOS
		bdos();
		break;
	default:
		break;
	}
}

void Board::bdos() {
	auto c = m_cpu.C();
	switch (c) {
	case 0x2: {
		auto character = m_cpu.E();
		std::cout << character;
		break;
	}
	case 0x9:
		for (uint16_t i = m_cpu.DE().word; peek(i) != '$'; ++i) {
			std::cout << peek(i);
		}
		break;
	}
}

void Board::Cpu_ExecutingInstruction_Profile(const EightBit::Z80& cpu) {

	const auto pc = m_cpu.PC();

	m_profiler.addAddress(pc.word);
	m_profiler.addInstruction(peek(pc.word));
}

void Board::Cpu_ExecutingInstruction_Debug(const EightBit::Z80& cpu) {

	std::cerr
		<< EightBit::Disassembler::state(m_cpu)
		<< "\t"
		<< m_disassembler.disassemble(m_cpu)
		<< '\n';
}
