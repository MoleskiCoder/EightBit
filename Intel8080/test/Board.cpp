#include "stdafx.h"
#include "Board.h"
#include "Configuration.h"

#include <iostream>

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::Intel8080(*this)),
  m_disassembler(*this) {
}

void Board::raisePOWER() noexcept {
	EightBit::Bus::raisePOWER();
	CPU().raisePOWER();
	CPU().raiseRESET();
	CPU().raiseINT();
}

void Board::lowerPOWER() noexcept {
	CPU().lowerPOWER();
	EightBit::Bus::lowerPOWER();
}

void Board::initialise() {

	auto romDirectory = m_configuration.getRomDirectory();

	//m_ram.load(romDirectory + "/TEST.COM", 0x100);		// Microcosm
	//m_ram.load(romDirectory + "/8080PRE.COM", 0x100);	// Bartholomew preliminary
	m_ram.load(romDirectory + "/8080EX1.COM", 0x100);	// Cringle/Bartholomew
	//m_ram.load(romDirectory + "/CPUTEST.COM", 0x100);	// SuperSoft diagnostics

	m_cpu.LoweredHALT.connect([this](EightBit::EventArgs) {
		lowerPOWER();
	});

	m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Cpm, this, std::placeholders::_1));

	if (m_configuration.isProfileMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	poke(0, 0xc3);	// JMP
	CPU().pokeWord(1, m_configuration.getStartAddress());

	poke(5, 0xc9); // ret
}

void Board::Cpu_ExecutingInstruction_Cpm(EightBit::Intel8080& cpu) {
	switch (cpu.PC().word) {
	case 0x0:	// CP/M warm start
		if (++m_warmstartCount == 2) {
			lowerPOWER();
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
	case 0x2: {
		const auto character = CPU().E();
		std::cout << character;
		break;
	}
	case 0x9:
		for (uint16_t i = CPU().DE().word; peek(i) != '$'; ++i) {
			std::cout << peek(i);
		}
		break;
	}
}

void Board::Cpu_ExecutingInstruction_Profile(EightBit::Intel8080& cpu) {

	const auto pc = cpu.PC();

	m_profiler.addAddress(pc.word);
	m_profiler.addInstruction(peek(pc.word));
}

void Board::Cpu_ExecutingInstruction_Debug(const EightBit::Intel8080& cpu) {
	const std::string disassembled = 
		EightBit::Disassembler::state(CPU()) + '\t' + m_disassembler.disassemble(CPU()) + '\n';
	std::cerr << disassembled;
}
