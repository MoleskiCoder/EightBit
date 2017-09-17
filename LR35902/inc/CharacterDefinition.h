#pragma once

#include <cstdint>
#include <vector>

#include <Bus.h>

namespace EightBit {
	namespace GameBoy {
		class CharacterDefinition {
		public:
			CharacterDefinition() {}

			CharacterDefinition(Bus& bus, uint16_t address, int height) {

				const int width = 8;

				m_definition.resize(width * height);

				for (auto row = 0; row < height; ++row) {

					auto planeAddress = address + row * 2;

					auto planeLow = bus.peek(planeAddress);
					auto planeHigh = bus.peek(planeAddress + 1);

					for (int bit = 0; bit < width; ++bit) {

						auto mask = 1 << bit;

						auto bitLow = planeLow & mask ? 1 : 0;
						auto bitHigh = planeHigh & mask ? 0b10 : 0;

						auto colour = bitHigh | bitLow;

						m_definition[row * width + ((width - 1) - bit)] = colour;
					}
				}
			}

			const std::vector<int>& get() const { return m_definition; }

		private:
			std::vector<int> m_definition;
		};
	}
}