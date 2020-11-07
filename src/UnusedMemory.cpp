#include "stdafx.h"
#include "UnusedMemory.h"

EightBit::UnusedMemory::UnusedMemory(const size_t size, const uint8_t value)
: m_size(size), m_value(value) {}

size_t EightBit::UnusedMemory::size() const {
	return m_size;
}

uint8_t EightBit::UnusedMemory::peek(uint16_t) const {
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

void EightBit::UnusedMemory::poke(uint16_t, uint8_t) {
	throw std::logic_error("Poke operation not allowed.");
}
