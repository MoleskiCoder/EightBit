#pragma once

#include <cstdint>
#include "Memory.h"

namespace EightBit {
	class Ram : public Memory {
	public:
		Ram(const size_t size = 0) noexcept
		: Memory(size) {
		}

		virtual uint8_t& reference(const uint16_t address) final {
			return BYTES()[address];
		}

		virtual void poke(const uint16_t address, const uint8_t value) final {
			Memory::poke(address, value);
		}
	};
}
