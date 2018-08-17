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

			uint8_t positionY() const { return m_positionY; }
			uint8_t positionX() const { return m_positionX; }
			uint8_t pattern() const { return m_pattern; }
			uint8_t flags() const { return m_flags; }

			uint8_t priority() const { return flags() & Processor::Bit7; }

			bool highPriority() const { return !!priority(); }
			bool lowPriority() const { return !priority(); }
			bool flipY() const { return !!(flags() & Processor::Bit6); }
			bool flipX() const { return !!(flags() & Processor::Bit5); }
			int palette() const { return (flags() & Processor::Bit4) >> 3; }

		private:
			uint8_t m_positionY;
			uint8_t m_positionX;
			uint8_t m_pattern;
			uint8_t m_flags;
		};
	}
}