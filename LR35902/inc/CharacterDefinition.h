#pragma once

#include <cstdint>
#include <array>

namespace EightBit {

	class Ram;

	namespace GameBoy {
		class CharacterDefinition final {
		public:
			CharacterDefinition(const Ram& vram, uint16_t address);

			std::array<int, 8> get(int row) const;

		private:
			const Ram& m_vram;
			uint16_t m_address = ~0;
		};
	}
}