#pragma once

#include <cstdint>
#include <array>
#include <limits>
#include "Chip.h"

namespace EightBit {

	class Memory;

	struct MemoryMapping final {
	private:
		static const int minimum = std::numeric_limits<std::uint16_t>::min();
		static const int maximum = std::numeric_limits<std::uint16_t>::max();

		std::array<uint16_t, maximum + 1> _offsets;

		constexpr auto create_offset(uint16_t address) const noexcept {
			return (address - begin) & mask;
		}

	public:
		enum class AccessLevel { Unknown, ReadOnly, WriteOnly, ReadWrite };

		Memory& memory;
		uint16_t begin = Chip::Mask16;
		uint16_t mask = 0U;
		AccessLevel access = AccessLevel::Unknown;

		MemoryMapping(Memory& memory, uint16_t begin, uint16_t mask, AccessLevel access) noexcept
		: memory(memory),
		  begin(begin),
		  mask(mask),
		  access(access) {
			for (int i = minimum; i < maximum + 1; ++i) {
				assert(i >= minimum);
				assert(i <= maximum);
				_offsets[i] = create_offset(i);
			}
		}

		constexpr auto offset(uint16_t address) const noexcept {
			return _offsets[address];
		}
	};
}
