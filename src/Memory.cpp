#include "stdafx.h"
#include "Memory.h"

#include <stdexcept>

uint8_t& EightBit::Memory::reference(uint16_t) {
	throw new std::logic_error("Reference operation not allowed.");
}
