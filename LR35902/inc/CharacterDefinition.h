#pragma once

#include <cstdint>
#include <array>

namespace EightBit {

	class Bus;

	namespace GameBoy {
		class CharacterDefinition {
		public:
			CharacterDefinition();
			CharacterDefinition(EightBit::Bus* bus, uint16_t address, int height);

			std::array<int, 8> get(int row) const;

		private:
			EightBit::Bus* m_bus;
			uint16_t m_address;
			int m_height;
		};
	}
}