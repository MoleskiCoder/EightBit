#include "stdafx.h"
#include "Ram.h"

EightBit::Ram::Ram(const size_t size) noexcept
: Rom(size) {}

uint8_t& EightBit::Ram::reference(const uint16_t address) {
	return BYTES()[address];
}

void EightBit::Ram::poke(const uint16_t address, const uint8_t value) {
	Rom::poke(address, value);
}
