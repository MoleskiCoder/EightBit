#include "stdafx.h"
#include "Board.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <assert.h>

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::mc6809(*this)),
  m_disassembler(m_cpu) {}

void Board::initialise() {

	const auto directory = m_configuration.getRomDirectory() + "\\";

	m_extendedBasic.load(directory + "extbas11.rom");
	m_colorBasic.load(directory + "bas12.rom");
	m_diskBasic.load(directory + "disk11.rom");

	if (m_configuration.isDebugMode()) {
		CPU().ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	CPU().powerOn();
	CPU().reset();
}

void Board::Cpu_ExecutingInstruction_Debug(EightBit::mc6809&) {
	const auto disassembled = m_disassembler.disassemble();
	if (!disassembled.empty())
		std::cout
			<< m_disassembler.dumpState()
			<< disassembled
			<< std::endl;
}

uint8_t& Board::reference(uint16_t address) {

	// 0x0000 - 0x7fff
	if (address < 0x8000)
		return m_ram.reference(address);

	// 0x8000 - 0x9fff
	if (address < 0xa000)
		return DATA() = m_extendedBasic.peek(address - 0x8000);

	// 0xa000 - 0xbfff
	if (address < 0xc000)
		return DATA() = m_colorBasic.peek(address - 0xa000);

	// 0xc000 - 0xdfff
	if (address < 0xe000)
		return DATA() = m_diskBasic.peek(address - 0xc000);

	// 0xe000 - 0xfeff
	if (address < 0xff00)
		return DATA() = 0xff;

	// 0xe000 - 0xfeff
	if (address < 0xfff0)
		return m_io.reference(address - 0xff00);

	// 0xfff0 - 0xffff
	const auto offset = address - 0xfff0;
	return DATA() = m_colorBasic.peek(0x1ff0 + offset);
}
