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

void Board::Cpu_ExecutingInstruction_Debug(EightBit::mc6809& cpu) {

	auto address = cpu.PC().word;
	auto cell = peek(address);

	const auto disassembled = m_disassembler.disassemble(address);
	if (!disassembled.empty()) {

		std::cout << std::hex;
		std::cout << "PC=" << EightBit::Disassembly::dump_WordValue(address) << ":";
		std::cout << "CC=" << EightBit::Disassembly::dump_Flags(CPU().CC()) << ",";
		std::cout << "D=" << EightBit::Disassembly::dump_WordValue(CPU().D().word) << ",";
		std::cout << "X=" << EightBit::Disassembly::dump_WordValue(CPU().X().word) << ",";
		std::cout << "Y=" << EightBit::Disassembly::dump_WordValue(CPU().Y().word) << ",";
		std::cout << "U=" << EightBit::Disassembly::dump_WordValue(CPU().U().word) << ",";
		std::cout << "S=" << EightBit::Disassembly::dump_WordValue(CPU().S().word) << ",";
		std::cout << "DP=" << EightBit::Disassembly::dump_ByteValue(CPU().DP()) << "\t";

		std::cout << disassembled << std::endl;
	}
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
