#pragma once

#include <cstdint>

#include <Chip.h>

namespace EightBit {

	class Ram;

	namespace GameBoy {
		class ObjectAttribute final {
		public:
			ObjectAttribute() = default;
			ObjectAttribute(Ram& ram, uint16_t address);

			[[nodiscard]] auto positionY() const { return m_positionY; }
			[[nodiscard]] auto positionX() const { return m_positionX; }
			[[nodiscard]] auto pattern() const { return m_pattern; }
			[[nodiscard]] auto flags() const { return m_flags; }

			[[nodiscard]] auto priority() const { return flags() & Chip::Bit7; }

			[[nodiscard]] auto highPriority() const { return !!priority(); }
			[[nodiscard]] auto lowPriority() const { return !priority(); }
			[[nodiscard]] auto flipY() const { return !!(flags() & Chip::Bit6); }
			[[nodiscard]] auto flipX() const { return !!(flags() & Chip::Bit5); }
			[[nodiscard]] auto palette() const { return (flags() & Chip::Bit4) >> 4; }

		private:
			uint8_t m_positionY;
			uint8_t m_positionX;
			uint8_t m_pattern;
			uint8_t m_flags;
		};
	}
}