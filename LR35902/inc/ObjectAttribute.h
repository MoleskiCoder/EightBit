#pragma once

#include <cstdint>

#include <Bus.h>
#include <Processor.h>

namespace EightBit {
	namespace GameBoy {
		class ObjectAttribute {
		public:
			ObjectAttribute() {}
			ObjectAttribute(Bus& bus, uint16_t address, int height) {
				m_positionY = bus.peek(address);
				m_positionX = bus.peek(address + 1);
				m_pattern = bus.peek(address + 2);
				if (height == 16)
					m_pattern >>= 1;
				m_flags = bus.peek(address + 3);
			}

			uint8_t positionY() const { return m_positionY; }
			uint8_t positionX() const { return m_positionX; }
			uint8_t pattern() const { return m_pattern; }
			uint8_t flags() const { return m_flags; }

			uint8_t priority() const { return flags() & Processor::Bit7; }

			bool highPriority() const { return priority() != 0; }
			bool lowPriority() const { return priority() == 0; }
			bool flipY() const { return (flags() & Processor::Bit6) != 0; }
			bool flipX() const { return (flags() & Processor::Bit5) != 0; }
			int palette() const { return (flags() & Processor::Bit4) >> 3; }

		private:
			uint8_t m_positionY;
			uint8_t m_positionX;
			uint8_t m_pattern;
			uint8_t m_flags;
		};
	}
}