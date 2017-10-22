#include "stdafx.h"
#include "CharacterDefinition.h"

#include <Ram.h>

EightBit::GameBoy::CharacterDefinition::CharacterDefinition()
: m_ram(nullptr),
  m_address(~0) {
}

EightBit::GameBoy::CharacterDefinition::CharacterDefinition(Ram* ram, uint16_t address)
: m_ram(ram),
  m_address(address) {
}

std::array<int, 8> EightBit::GameBoy::CharacterDefinition::get(int row) const {

	std::array<int, 8> returned;

	auto planeAddress = m_address + row * 2;
	
	auto planeLow = m_ram->peek(planeAddress);
	auto planeHigh = m_ram->peek(planeAddress + 1);
	
	for (int bit = 0; bit < 8; ++bit) {
	
		auto mask = 1 << bit;
	
		auto bitLow = planeLow & mask ? 1 : 0;
		auto bitHigh = planeHigh & mask ? 0b10 : 0;
	
		auto colour = bitHigh | bitLow;
	
		returned[7 - bit] = colour;
	}

	return returned;
}
