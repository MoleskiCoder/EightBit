#include "stdafx.h"
#include "../inc/ObjectAttribute.h"

#include <Ram.h>

EightBit::GameBoy::ObjectAttribute::ObjectAttribute(Ram& ram, uint16_t address) {
	m_positionY = ram.peek(address);
	m_positionX = ram.peek(++address);
	m_pattern = ram.peek(++address);
	m_flags = ram.peek(++address);
}
