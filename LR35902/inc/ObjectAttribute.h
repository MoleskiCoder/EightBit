#pragma once

#include <cstdint>

#include <Processor.h>

namespace EightBit {

	class Ram;

	namespace GameBoy {
		class ObjectAttribute final {
		public:
			ObjectAttribute() = default;
			ObjectAttribute(Ram& ram, uint16_t address);

			auto positionY() const { return m_positionY; }
			auto positionX() const { return m_positionX; }
			auto pattern() const { return m_pattern; }
			auto flags() const { return m_flags; }

			auto priority() const { return flags() & Chip::Bit7; }

			auto highPriority() const { return !!priority(); }
			auto lowPriority() const { return !priority(); }
			auto flipY() const { return !!(flags() & Chip::Bit6); }
			auto flipX() const { return !!(flags() & Chip::Bit5); }
			auto palette() const { return (flags() & Chip::Bit4) >> 3; }

		private:
			uint8_t m_positionY;
			uint8_t m_positionX;
			uint8_t m_pattern;
			uint8_t m_flags;
		};
	}
}