#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration) {}

void Board::powerOn() {
	EightBit::Bus::powerOn();
	CPU().powerOn();
	CPU().reset();
}

void Board::powerOff() {
	CPU().powerOff();
	EightBit::Bus::powerOff();
}

void Board::initialise() {

	auto romDirectory = m_configuration.getRomDirectory();
	m_ram.load(romDirectory + "/zexall.com", 0x100);	// Cringle/Bartholomew

	m_cpu.ExecutingInstruction.connect([this] (EightBit::Z80& cpu) {
		if (UNLIKELY(EightBit::Chip::lowered(cpu.HALT())))
			powerOff();
		switch (cpu.PC().word) {
		case 0x0:	// CP/M warm start
			if (++m_warmstartCount == 3) {
				powerOff();
				if (m_configuration.isProfileMode())
					m_profiler.dump();
			}
			break;
		case 0x5:	// BDOS
			bdos();
			break;
		default:
			break;
		}
	});

	if (m_configuration.isProfileMode())
		m_cpu.ExecutingInstruction.connect([this] (EightBit::Z80& cpu) {
			const auto pc = cpu.PC();
			m_profiler.addAddress(pc.word);
			m_profiler.addInstruction(peek(pc));
		});

	if (m_configuration.isDebugMode())
		m_cpu.ExecutingInstruction.connect([this] (EightBit::Z80& cpu) {
			std::cerr
				<< EightBit::Disassembler::state(cpu)
				<< "\t"
				<< m_disassembler.disassemble(cpu)
				<< std::endl;
		});

	poke(0, 0xc3);	// JMP
	CPU().pokeWord(1, m_configuration.getStartAddress());
	poke(5, 0xc9);	// ret
}

void Board::bdos() {
	switch (CPU().C()) {
	case 0x2:
		std::cout << CPU().E();
		break;
	case 0x9:
		for (uint16_t i = CPU().DE().word; peek(i) != '$'; ++i)
			std::cout << peek(i);
		break;
	}
}
