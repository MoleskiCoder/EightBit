#pragma once

#include <cstdint>

#include "Rom.h"

namespace EightBit {
	// The RAM class is everything the ROM class is, plus
	// it's externally 'reference'able and 'poke'able.
	class Ram : public Rom {
	public:
		Ram(size_t size = 0) noexcept;

		virtual uint8_t& reference(uint16_t address) final;
		virtual void poke(uint16_t address, uint8_t value) final;
	};
}
