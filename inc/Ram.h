#pragma once

#include <cstdint>

#include "Rom.h"

namespace EightBit {
	// The RAM class is everything the ROM class is, plus
	// it's externally 'reference'able and 'poke'able.
	class Ram : public Rom {
	public:
		Ram(const size_t size = 0) noexcept
		: Rom(size) {
		}

		virtual uint8_t& reference(const uint16_t address) final {
			return BYTES()[address];
		}

		virtual void poke(const uint16_t address, const uint8_t value) final {
			Rom::poke(address, value);
		}
	};
}
