#pragma once

#include <cstdint>
#include <array>

#include "Bus.h"

namespace EightBit {
	namespace GameBoy {
		class CharacterDefinition {
		public:
			CharacterDefinition() {}

			CharacterDefinition(Bus& bus, uint16_t address) {

				for (auto row = 0; row < 8; ++row) {

					auto planeAddress = address + row * 2;

					auto planeLow = bus.peek(planeAddress);
					auto planeHigh = bus.peek(planeAddress + 1);

					for (int bit = 0; bit < 8; ++bit) {

						auto mask = 1 << bit;

						auto bitLow = planeLow & mask ? 1 : 0;
						auto bitHigh = planeHigh & mask ? 0b10 : 0;

						auto colour = bitHigh | bitLow;

						m_definition[row * 8 + (7 - bit)] = colour;
					}
				}
			}

			const std::array<int, 8 * 8>& get() const { return m_definition; }

		private:
			std::array<int, 8 * 8> m_definition;
		};
	}
}