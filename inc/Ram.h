#pragma once

#include <cstdint>
#include "Memory.h"

namespace EightBit {
	class Ram : public Memory {
	public:
		Ram(const size_t size = 0)
		: Memory(size) {
		}

		uint8_t peek(const uint16_t address) const {
			return read(address);
		}

		void poke(const uint16_t address, const uint8_t value) {
			write(address, value);
		}

		uint8_t& reference(const uint16_t address) {
			return BYTES()[address];
		}
	};
}
