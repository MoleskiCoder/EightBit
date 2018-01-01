#include "stdafx.h"
#include "Ricoh2A03.h"

EightBit::Ricoh2A03::Ricoh2A03(Bus& bus)
: MOS6502(bus) {
}

uint8_t EightBit::Ricoh2A03::SUB(uint8_t operand, uint8_t data, int borrow) {
	return MOS6502::SUB_b(operand ,data, borrow);
}

void EightBit::Ricoh2A03::ADC(uint8_t data) {
	MOS6502::ADC_b(data);
}
