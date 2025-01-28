#include "stdafx.h"
#include "../inc/CharacterDefinition.h"

#include <Ram.h>

EightBit::GameBoy::CharacterDefinition::CharacterDefinition(Ram& vram, const uint16_t address) noexcept
: m_vram(vram),
  m_address(address) {
}

std::array<int, 8> EightBit::GameBoy::CharacterDefinition::get(int row) const noexcept {

	std::array<int, 8> returned {};

	const auto planeAddress = m_address + row * 2;
	
	const auto planeLow = m_vram.peek(planeAddress);
	const auto planeHigh = m_vram.peek(planeAddress + 1);
	
	for (int bit = 0; bit < 8; ++bit) {
	
		const auto mask = Chip::bit(bit);
	
		const auto bitLow = planeLow & mask ? 1 : 0;
		const auto bitHigh = planeHigh & mask ? 0b10 : 0;
	
		const auto colour = bitHigh | bitLow;
	
		const auto index = 7 - bit;
		returned[index] = colour;
	}

	return returned;
}
