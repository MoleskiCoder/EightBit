#include "stdafx.h"
#include "Device.h"

void EightBit::Device::powerOn() {
	raise(POWER());
}

void EightBit::Device::match(PinLevel& line, int value) {
	value ? raise(line) : lower(line);
}
