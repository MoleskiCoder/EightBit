#pragma once

#include "Rom.h"

namespace EightBit {
	// The RAM class is everything the ROM class is, plus
	// it's externally 'reference'able and 'poke'able.
	class Ram : public Rom {
	public:
		Ram(size_t size = 0) noexcept;

		[[nodiscard]] uint8_t& reference(uint16_t address) noexcept final;
		void poke(uint16_t address, uint8_t value) noexcept final;
	};
}
