#include "stdafx.h"
#include "Memory.h"
#include "Processor.h"

#include <iostream>
#include <fstream>
#include <algorithm>

EightBit::Memory::Memory(uint16_t addressMask)
: m_addressMask(addressMask),
  m_temporary(0) {
	clear();
	m_address.word = 0;
	m_data = &(m_bus[m_address.word]);
}

EightBit::Memory::~Memory() {
}

uint8_t EightBit::Memory::peek(uint16_t address) const {
	return m_bus[address];
}

uint16_t EightBit::Memory::peekWord(uint16_t address) const {
	register16_t returned;
	returned.low = peek(address);
	returned.high = peek(address + 1);
	return returned.word;
}

void EightBit::Memory::clear() {
	m_bus.fill(0);
	m_locked.fill(false);
}

void EightBit::Memory::loadRom(const std::string& path, uint16_t offset) {
	auto size = loadMemory(path, offset);
	std::fill(m_locked.begin() + offset, m_locked.begin() + offset + size, true);
}

void EightBit::Memory::loadRam(const std::string& path, uint16_t offset) {
	loadMemory(path, offset);
}

int EightBit::Memory::loadMemory(const std::string& path, uint16_t offset) {
	std::ifstream file;
	file.exceptions(std::ios::failbit | std::ios::badbit);

	file.open(path, std::ios::binary | std::ios::ate);
	auto size = (int)file.tellg();

	size_t extent = size + offset;

	if (extent > m_bus.size())
		throw std::runtime_error("ROM cannot fit");

	file.seekg(0, std::ios::beg);

	file.read((char*)&m_bus[offset], size);
	file.close();

	return size;
}
