#pragma once

#include <cstdint>
#include <array>

namespace EightBit {

	class Ram;

	namespace GameBoy {
		class CharacterDefinition {
		public:
			CharacterDefinition() = default;
			CharacterDefinition(Ram* ram, uint16_t address);

			std::array<int, 8> get(int row) const;

		private:
			Ram* m_ram = nullptr;
			uint16_t m_address = ~0;
		};
	}
}