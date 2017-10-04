#pragma once

#include <cstdint>
#include <array>

namespace EightBit {

	class Ram;

	namespace GameBoy {
		class CharacterDefinition {
		public:
			CharacterDefinition();
			CharacterDefinition(Ram* ram, uint16_t address, int height);

			std::array<int, 8> get(int row) const;

		private:
			Ram* m_ram;
			uint16_t m_address;
			int m_height;
		};
	}
}