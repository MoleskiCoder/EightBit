#include "stdafx.h"
#include "../inc/Ricoh2A03.h"

EightBit::Ricoh2A03::Ricoh2A03(Bus& bus)
: MOS6502(bus) {
}

uint8_t EightBit::Ricoh2A03::sub(uint8_t operand, uint8_t data, int borrow) {
	return MOS6502::sub_b(operand ,data, borrow);
}

uint8_t EightBit::Ricoh2A03::add(uint8_t operand, uint8_t data, int carry) {
	return MOS6502::add_b(operand, data, carry);
}
