#include "pch.h"
#include "Board.h"

Board::Board()
: m_cpu(EightBit::mc6809(*this)),
  m_disassembler(*this, m_cpu) {
}

void Board::raisePOWER() {
	EightBit::Bus::raisePOWER();

	CPU().raisePOWER();

	CPU().lowerRESET();
	CPU().raiseINT();

	CPU().raiseNMI();
	CPU().raiseFIRQ();
	CPU().raiseHALT();
}

void Board::lowerPOWER() {
	CPU().lowerPOWER();
	EightBit::Bus::lowerPOWER();
}

void Board::initialise() {
	//CPU().ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	//CPU().ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_Debug, this, std::placeholders::_1));
}

EightBit::MemoryMapping Board::mapping(uint16_t) {
	return { m_ram, 0x0000, 0xffff, EightBit::MemoryMapping::AccessLevel::ReadWrite };
}

void Board::Cpu_ExecutingInstruction_Debug(EightBit::mc6809&) {
	m_disassembleAt = CPU().PC();
	m_ignoreDisassembly = m_disassembler.ignore();
}

void Board::Cpu_ExecutedInstruction_Debug(EightBit::mc6809&) {
	if (!m_ignoreDisassembly)
		std::cout << m_disassembler.trace(m_disassembleAt) << "	" << std::endl;
}
