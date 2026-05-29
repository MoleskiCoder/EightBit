#include "stdafx.h"
#include "../inc/Ram.h"

EightBit::Ram::Ram(const size_t size) noexcept
: Rom(size) {}

void EightBit::Ram::poke(const uint16_t address, const uint8_t value) noexcept {
	Rom::poke(address, value);
}
