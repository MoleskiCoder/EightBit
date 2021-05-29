#include "stdafx.h"
#include "../inc/Memory.h"

#include <stdexcept>

uint8_t& EightBit::Memory::reference(uint16_t) {
	throw std::logic_error("Reference operation not allowed.");
}
