#include "stdafx.h"
#include "../inc/UnusedMemory.h"

#include <cassert>

EightBit::UnusedMemory::UnusedMemory(const uint16_t size, const uint8_t value) noexcept
: m_size(size), m_value(value) {}

uint16_t EightBit::UnusedMemory::size() const noexcept {
	return m_size;
}

uint8_t EightBit::UnusedMemory::peek(uint16_t) const noexcept {
	return m_value;
}

int EightBit::UnusedMemory::load(std::ifstream&, int, int, int) {
	throw std::logic_error("load operation not allowed.");
}

int EightBit::UnusedMemory::load(std::string, int, int, int) {
	throw std::logic_error("load operation not allowed.");
}
		
int EightBit::UnusedMemory::load(const std::vector<uint8_t>&, int, int, int) {
	throw std::logic_error("load operation not allowed.");
}

void EightBit::UnusedMemory::poke(uint16_t, uint8_t) noexcept {
	assert(false && "Poke operation not allowed.");
}
