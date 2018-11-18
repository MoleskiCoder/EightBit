#pragma once

#include <cstdint>
#include "Chip.h"

namespace EightBit {

	class Memory;

	struct MemoryMapping {

		enum AccessLevel { Unknown, ReadOnly, ReadWrite, };

		Memory& memory;
		uint16_t begin = Chip::Mask16;
		uint16_t mask = 0U;
		AccessLevel access = Unknown;
	};
}
