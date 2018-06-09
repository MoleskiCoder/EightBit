#pragma once

#include <cstdint>
#include "Memory.h"

namespace EightBit {
	class Ram : public Memory {
	public:
		Ram(const size_t size = 0)
		: Memory(size) {
		}

		uint8_t& reference(const uint16_t address) {
			return BYTES()[address];
		}

		uint8_t reference(uint16_t address) const {
			return peek(address);
		}

		void poke(uint16_t address, uint8_t value) {
			Memory::poke(address, value);
		}
	};
}
