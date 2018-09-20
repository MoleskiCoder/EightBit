#pragma once

#include <cstdint>
#include "Memory.h"

namespace EightBit {
	class Ram : public Memory {
	public:
		Ram(const size_t size = 0) noexcept
		: Memory(size) {
		}

		uint8_t& reference(const uint16_t address) {
			return BYTES()[address];
		}

		void poke(const uint16_t address, const uint8_t value) {
			Memory::poke(address, value);
		}
	};
}
