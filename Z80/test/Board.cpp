#include "stdafx.h"
#include "Board.h"

#include <Disassembler.h>

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::Z80(*this, m_ports)),
  m_disassembler(*this),
  m_profiler(m_cpu, m_disassembler) {
}

void Board::initialise() {

	auto romDirectory = m_configuration.getRomDirectory();

	//m_ram.load(romDirectory + "/prelim.com", 0x100);	// Bartholomew preliminary
	//m_ram.load(romDirectory + "/zexdoc.com", 0x100);	// Cringle/Bartholomew
	m_ram.load(romDirectory + "/zexall.com", 0x100);	// Cringle/Bartholomew
	//m_ram.load(romDirectory + "/CPUTEST.COM", 0x100);	// SuperSoft diagnostics

	m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Cpm, this, std::placeholders::_1));

	if (m_configuration.isProfileMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	CPU().powerOn();
	CPU().reset();

	poke(0, 0xc3);	// JMP
	poke(1, m_configuration.getStartAddress().low);
	poke(2, m_configuration.getStartAddress().high);

	poke(5, 0xc9);	// ret
}

void Board::Cpu_ExecutingInstruction_Cpm(EightBit::Z80& cpu) {
	if (UNLIKELY(EightBit::Chip::lowered(cpu.HALT())))
		CPU().powerOff();
	switch (cpu.PC().word) {
	case 0x0:	// CP/M warm start
		if (++m_warmstartCount == 3) {
			CPU().powerOff();
			if (m_configuration.isProfileMode()) {
				m_profiler.dump();
			}
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
	switch (CPU().C()) {
	case 0x2:
		std::cout << CPU().E();
		break;
	case 0x9:
		for (uint16_t i = CPU().DE().word; peek(i) != '$'; ++i) {
			std::cout << peek(i);
		}
		break;
	}
}

void Board::Cpu_ExecutingInstruction_Profile(EightBit::Z80& cpu) {

	const auto pc = cpu.PC();

	m_profiler.addAddress(pc.word);
	m_profiler.addInstruction(peek(pc));
}

void Board::Cpu_ExecutingInstruction_Debug(EightBit::Z80& cpu) {

	std::cerr
		<< EightBit::Disassembler::state(cpu)
		<< "\t"
		<< m_disassembler.disassemble(cpu)
		<< std::endl;
}
