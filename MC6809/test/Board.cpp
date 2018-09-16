#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::mc6809(*this)),
  m_disassembler(m_cpu) {
	std::vector<uint8_t> content(m_unused2000.size());
	std::fill(content.begin(), content.end(), 0xff);
	m_unused2000.load(content);
}

void Board::initialise() {

	const auto directory = m_configuration.getRomDirectory() + "\\";

	loadHexFile(directory + "ExBasROM.hex");

	if (m_configuration.isDebugMode()) {
		CPU().ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
		CPU().ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_Debug, this, std::placeholders::_1));
	}

	CPU().powerOn();
	CPU().raise(CPU().NMI());
	CPU().raise(CPU().FIRQ());
	CPU().reset();
}

void Board::Cpu_ExecutingInstruction_Debug(EightBit::mc6809&) {
	m_disassembleAt = CPU().PC();
	m_ignoreDisassembly = m_disassembler.ignore();
}

void Board::Cpu_ExecutedInstruction_Debug(EightBit::mc6809&) {
	if (!m_ignoreDisassembly)
		std::cout << m_disassembler.trace(m_disassembleAt) << std::endl;
}

EightBit::MemoryMapping Board::mapping(uint16_t address) {

	if (address < 0x8000)
		return { m_ram, 0x0000, EightBit::MemoryMapping::ReadWrite };

	if (address < 0xa000)
		return { m_unused2000, 0x8000, EightBit::MemoryMapping::ReadOnly };

	if (address < 0xc000)
		return { m_io, 0xa000, EightBit::MemoryMapping::ReadWrite };

	return { m_rom, 0xc000, EightBit::MemoryMapping::ReadOnly };
}
