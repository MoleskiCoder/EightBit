#include "stdafx.h"
#include "ObjectAttribute.h"

#include <Ram.h>

EightBit::GameBoy::ObjectAttribute::ObjectAttribute() {
}

EightBit::GameBoy::ObjectAttribute::ObjectAttribute(Ram& ram, uint16_t address, int height) {
	m_positionY = ram.peek(address);
	m_positionX = ram.peek(address + 1);
	m_pattern = ram.peek(address + 2);
	if (height == 16)
		m_pattern >>= 1;
	m_flags = ram.peek(address + 3);
}
