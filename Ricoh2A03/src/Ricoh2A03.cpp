#include "stdafx.h"
#include "../inc/Ricoh2A03.h"

EightBit::Ricoh2A03::Ricoh2A03(Bus& bus)
: MOS6502(bus) {
}

uint8_t EightBit::Ricoh2A03::sub(uint8_t operand, int borrow) noexcept {
	return MOS6502::sub_b(operand, borrow);
}

void EightBit::Ricoh2A03::adc() noexcept {
	MOS6502::adc_b();
}
