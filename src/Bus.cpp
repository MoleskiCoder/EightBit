#include "stdafx.h"
#include "Bus.h"

#include "EightBitCompilerDefinitions.h"

uint8_t EightBit::Bus::peek(const uint16_t address) const {
	bool rom;
	return reference(address, rom);
}

void EightBit::Bus::poke(const uint16_t address, const uint8_t value) {
	bool rom;
	reference(address, rom) = value;
}

uint16_t EightBit::Bus::peekWord(const uint16_t address) const {
	register16_t returned;
	returned.low = peek(address);
	returned.high = peek(address + 1);
	return returned.word;
}

uint8_t EightBit::Bus::read() {
	ReadingByte.fire(ADDRESS().word);
	DATA() = reference();
	ReadByte.fire(ADDRESS().word);
	return DATA();
}

uint8_t EightBit::Bus::read(const uint16_t offset) {
	ADDRESS().word = offset;
	return read();
}

uint8_t EightBit::Bus::read(const register16_t address) {
	ADDRESS() = address;
	return read();
}

void EightBit::Bus::write() {
	WritingByte.fire(ADDRESS().word);
	reference() = DATA();
	WrittenByte.fire(ADDRESS().word);
}

void EightBit::Bus::write(const uint8_t value) {
	DATA() = value;
	write();
}

void EightBit::Bus::write(const uint16_t offset, const uint8_t value) {
	ADDRESS().word = offset;
	write(value);
}

void EightBit::Bus::write(const register16_t address, const uint8_t value) {
	ADDRESS() = address;
	write(value);
}

uint8_t EightBit::Bus::reference() const {
	bool rom;
	return reference(ADDRESS().word, rom);
}

uint8_t& EightBit::Bus::reference() {
	bool rom;
	return reference(ADDRESS().word, rom);
}
