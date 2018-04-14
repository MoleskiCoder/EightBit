#pragma once

#include <cstdint>
#include "Memory.h"

namespace EightBit {
	class Rom : public Memory {
	public:
		Rom(const size_t size = 0)
		: Memory(size) {
		}

		uint8_t peek(const uint16_t address) const {
			return read(address);
		}

		uint8_t& reference(const uint16_t address) {
			return BYTES()[address];
		}
	};
}
