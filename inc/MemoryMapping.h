#pragma once

#include <cstdint>
#include "Chip.h"

namespace EightBit {

	class Memory;

	struct MemoryMapping final {

		enum class AccessLevel { Unknown, ReadOnly, WriteOnly, ReadWrite };

		constexpr auto offset(uint16_t address) const noexcept { return (address - begin) & mask; }

		Memory& memory;
		uint16_t begin = Chip::Mask16;
		uint16_t mask = 0U;
		AccessLevel access = AccessLevel::Unknown;
	};
}
