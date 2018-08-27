#include "stdafx.h"
#include "Bus.h"

uint8_t EightBit::Bus::read() {
	ReadingByte.fire(EventArgs::empty());
	DATA() = reference();
	ReadByte.fire(EventArgs::empty());
	return DATA();
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
