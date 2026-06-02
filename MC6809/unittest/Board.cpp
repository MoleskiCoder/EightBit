#include "pch.h"
#include "Board.h"

Board::Board() {}

void Board::raisePOWER() noexcept {
	EightBit::Bus::raisePOWER();

	CPU().raisePOWER();

	CPU().lowerRESET();
	CPU().raiseINT();

	CPU().raiseNMI();
	CPU().raiseFIRQ();
	CPU().raiseHALT();
}

void Board::lowerPOWER() noexcept {
	CPU().lowerPOWER();
	EightBit::Bus::lowerPOWER();
}

void Board::initialise() noexcept {
	CPU().ExecutingInstruction.connect([this](EightBit::mc6809& cpu) {
		m_disassembleAt = CPU().PC();
		m_ignoreDisassembly = m_disassembler.ignore();
	});

	CPU().ExecutedInstruction.connect([this](EightBit::mc6809& cpu) {
		if (!m_ignoreDisassembly)
			std::cout << m_disassembler.trace(m_disassembleAt) << "	" << std::endl;
	});
}

const EightBit::MemoryMapping& Board::mapping(uint16_t) noexcept {
	return m_mapping;
}
