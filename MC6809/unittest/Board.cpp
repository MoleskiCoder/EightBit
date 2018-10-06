#include "pch.h"
#include "Board.h"

Board::Board()
: m_cpu(EightBit::mc6809(*this)) {
}

void Board::initialise() {
	CPU().powerOn();
	CPU().raise(CPU().NMI());
	CPU().raise(CPU().FIRQ());
	CPU().reset();
}

EightBit::MemoryMapping Board::mapping(uint16_t) {
	return { m_ram, 0x0000, EightBit::MemoryMapping::ReadWrite };
}
