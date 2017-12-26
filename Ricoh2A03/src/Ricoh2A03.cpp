#include "stdafx.h"
#include "Ricoh2A03.h"

EightBit::Ricoh2A03::Ricoh2A03(Bus& bus)
: MOS6502(bus) {
}

void EightBit::Ricoh2A03::SBC(uint8_t data) {
	MOS6502::SBC_b(data);
}

void EightBit::Ricoh2A03::ADC(uint8_t data) {
	MOS6502::ADC_b(data);
}
