#include "stdafx.h"
#include "Bus.h"

#include "EightBitCompilerDefinitions.h"

EightBit::register16_t& EightBit::Bus::ADDRESS() {
	return m_address;
}

uint8_t& EightBit::Bus::DATA() {
	return *m_data;
}

uint8_t& EightBit::Bus::placeDATA(const uint8_t value) {
	m_temporary = value;
	m_data = &m_temporary;
	return DATA();
}

uint8_t& EightBit::Bus::referenceDATA(uint8_t& value) {
	m_data = &value;
	return DATA();
}

uint8_t EightBit::Bus::peek(const uint16_t address) {
	bool rom;
	return reference(address, rom);
}

void EightBit::Bus::poke(const uint16_t address, const uint8_t value) {
	bool rom;
	reference(address, rom) = value;
}

uint16_t EightBit::Bus::peekWord(const uint16_t address) {
	register16_t returned;
	returned.low = peek(address);
	returned.high = peek(address + 1);
	return returned.word;
}

uint8_t EightBit::Bus::read() {
	ReadingByte.fire(ADDRESS().word);
	const auto returned = reference();
	ReadByte.fire(ADDRESS().word);
	return returned;
}

uint8_t EightBit::Bus::read(const uint16_t offset) {
	ADDRESS().word = offset;
	return read();
}

uint8_t EightBit::Bus::read(const register16_t address) {
	ADDRESS() = address;
	return read();
}

void EightBit::Bus::write(const uint8_t value) {
	WritingByte.fire(ADDRESS().word);
	reference() = value;
	WrittenByte.fire(ADDRESS().word);
}

void EightBit::Bus::write(const uint16_t offset, const uint8_t value) {
	ADDRESS().word = offset;
	write(value);
}

void EightBit::Bus::write(const register16_t address, const uint8_t value) {
	ADDRESS() = address;
	write(value);
}

uint8_t& EightBit::Bus::reference() {
	bool rom;
	auto& value = reference(ADDRESS().word, rom);
	return rom ? placeDATA(value) : referenceDATA(value);
}
