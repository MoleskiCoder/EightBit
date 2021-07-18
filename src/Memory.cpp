#include "stdafx.h"
#include "../inc/Memory.h"

#include <cassert>

uint8_t& EightBit::Memory::reference(uint16_t) noexcept {
	assert(false && "Reference operation not allowed.");
	return m_delivered;
}
