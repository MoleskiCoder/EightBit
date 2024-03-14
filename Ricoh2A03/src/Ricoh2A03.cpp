#include "stdafx.h"
#include "../inc/Ricoh2A03.h"

EightBit::Ricoh2A03::Ricoh2A03(Bus& bus)
: MOS6502(bus) {
}

uint8_t EightBit::Ricoh2A03::sub(uint8_t operand, int borrow) noexcept {
	const auto data = BUS().DATA();
	return MOS6502::sub_b(operand ,data, borrow);
}

uint8_t EightBit::Ricoh2A03::add(uint8_t operand, int carry) noexcept {
	const auto data = BUS().DATA();
	return MOS6502::add_b(operand, data, carry);
}
