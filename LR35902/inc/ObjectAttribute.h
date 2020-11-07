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

			[[nodiscard]] auto positionY() const noexcept { return m_positionY; }
			[[nodiscard]] auto positionX() const noexcept { return m_positionX; }
			[[nodiscard]] auto pattern() const noexcept { return m_pattern; }
			[[nodiscard]] auto flags() const noexcept { return m_flags; }

			[[nodiscard]] auto priority() const noexcept { return flags() & Chip::Bit7; }

			[[nodiscard]] auto highPriority() const noexcept { return !!priority(); }
			[[nodiscard]] auto lowPriority() const noexcept { return !priority(); }
			[[nodiscard]] auto flipY() const noexcept { return !!(flags() & Chip::Bit6); }
			[[nodiscard]] auto flipX() const noexcept { return !!(flags() & Chip::Bit5); }
			[[nodiscard]] auto palette() const noexcept { return (flags() & Chip::Bit4) >> 4; }

		private:
			uint8_t m_positionY;
			uint8_t m_positionX;
			uint8_t m_pattern;
			uint8_t m_flags;
		};
	}
}