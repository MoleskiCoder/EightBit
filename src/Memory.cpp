#include "stdafx.h"
#include "Memory.h"
#include "Processor.h"

#include <iostream>
#include <fstream>
#include <algorithm>

EightBit::Memory::Memory(uint16_t addressMask)
: m_addressMask(addressMask),
  m_temporary(0),
  m_data(nullptr) {
	clear();
	m_address.word = 0;
	m_bus.resize(0x10000);
	m_locked.resize(0x10000);
}

EightBit::Memory::~Memory() {
}

uint8_t EightBit::Memory::peek(uint16_t address) {
	bool rom;
	return reference(address, rom);
}

void EightBit::Memory::poke(uint16_t address, uint8_t value) {
	bool rom;
	reference(address, rom) = value;
}

uint16_t EightBit::Memory::peekWord(uint16_t address) {
	register16_t returned;
	returned.low = peek(address);
	returned.high = peek(address + 1);
	return returned.word;
}

void EightBit::Memory::clear() {
	std::fill(m_bus.begin(), m_bus.end(), 0);
	std::fill(m_locked.begin(), m_locked.end(), false);
}

void EightBit::Memory::loadRom(const std::string& path, uint16_t offset) {
	auto size = loadMemory(path, offset);
	lock(offset, size);
}

void EightBit::Memory::loadRam(const std::string& path, uint16_t offset) {
	loadMemory(path, offset);
}

int EightBit::Memory::loadMemory(const std::string& path, uint16_t offset) {
	return loadBinary(path, m_bus, offset, 0x10000 - offset);
}

int EightBit::Memory::loadBinary(const std::string& path, std::vector<uint8_t>& output, int offset, int maximumSize) {
	std::ifstream file;
	file.exceptions(std::ios::failbit | std::ios::badbit);

	file.open(path, std::ios::binary | std::ios::ate);
	auto size = (int)file.tellg();
	if ((maximumSize > 0) && (size > maximumSize))
		throw std::runtime_error("Binary cannot fit");

	size_t extent = size + offset;
	if (output.size() < extent)
		output.resize(extent);

	file.seekg(0, std::ios::beg);

	file.read((char*)&output[offset], size);
	file.close();

	return size;
}