#include "stdafx.h"
#include "CharacterDefinition.h"

#include <Bus.h>

EightBit::GameBoy::CharacterDefinition::CharacterDefinition()
: m_bus(nullptr),
  m_address(~0),
  m_height(0) {
}

EightBit::GameBoy::CharacterDefinition::CharacterDefinition(EightBit::Bus* bus, uint16_t address, int height)
: m_bus(bus),
  m_address(address),
  m_height(height) {
}

std::array<int, 8> EightBit::GameBoy::CharacterDefinition::get(int row) const {

	std::array<int, 8> returned;

	auto planeAddress = m_address + row * 2;
	
	auto planeLow = m_bus->peek(planeAddress);
	auto planeHigh = m_bus->peek(planeAddress + 1);
	
	for (int bit = 0; bit < 8; ++bit) {
	
		auto mask = 1 << bit;
	
		auto bitLow = planeLow & mask ? 1 : 0;
		auto bitHigh = planeHigh & mask ? 0b10 : 0;
	
		auto colour = bitHigh | bitLow;
	
		returned[7 - bit] = colour;
	}

	return returned;
}
