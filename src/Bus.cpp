#include "stdafx.h"
#include "Bus.h"

uint8_t EightBit::Bus::peek() const {
	return reference();
}

uint8_t EightBit::Bus::peek(const uint16_t address) const {
	return reference(address);
}

void EightBit::Bus::poke(const uint8_t value) {
	reference() = value;
}

void EightBit::Bus::poke(const uint16_t address, const uint8_t value) {
	reference(address) = value;
}

uint16_t EightBit::Bus::peekWord(const uint16_t address) const {
	register16_t returned;
	returned.low = peek(address);
	returned.high = peek(address + 1);
	return returned.word;
}

uint8_t EightBit::Bus::read() {
	ReadingByte.fire(EventArgs::empty());
	DATA() = reference();
	ReadByte.fire(EventArgs::empty());
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
	WritingByte.fire(EventArgs::empty());
	reference() = DATA();
	WrittenByte.fire(EventArgs::empty());
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
	return reference(ADDRESS().word);
}

uint8_t& EightBit::Bus::reference() {
	return reference(ADDRESS().word);
}
