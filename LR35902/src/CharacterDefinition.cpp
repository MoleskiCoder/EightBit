#include "stdafx.h"
#include "CharacterDefinition.h"

#include <Ram.h>

EightBit::GameBoy::CharacterDefinition::CharacterDefinition(gsl::not_null<Ram*> ram, uint16_t address)
: m_ram(ram),
  m_address(address) {
}

std::array<int, 8> EightBit::GameBoy::CharacterDefinition::get(int row) const {

	std::array<int, 8> returned;

	const auto planeAddress = m_address + row * 2;
	
	const auto planeLow = m_ram->peek(planeAddress);
	const auto planeHigh = m_ram->peek(planeAddress + 1);
	
	for (int bit = 0; bit < 8; ++bit) {
	
		const auto mask = 1 << bit;
	
		const auto bitLow = planeLow & mask ? 1 : 0;
		const auto bitHigh = planeHigh & mask ? 0b10 : 0;
	
		const auto colour = bitHigh | bitLow;
	
		returned[7 - bit] = colour;
	}

	return returned;
}
