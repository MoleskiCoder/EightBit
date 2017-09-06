#pragma once

#include <cstdint>
#include "Memory.h"

namespace EightBit {
	class Ram : public Memory {
	public:
		Ram(size_t size)
		: Memory(size) {
		}

		uint8_t peek(uint16_t address) const {
			return read(address);
		}

		void poke(uint16_t address, uint8_t value) {
			write(address, value);
		}

		uint8_t& reference(uint16_t address) {
			return m_bytes[address];
		}
	};
}
