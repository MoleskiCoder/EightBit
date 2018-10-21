#include "stdafx.h"
#include "Chip.h"

void EightBit::Chip::powerOn() {
	raise(POWER());
}

void EightBit::Chip::match(PinLevel& line, int value) {
	value ? raise(line) : lower(line);
}
