#pragma once

#include <cstdint>

#include "Memory.h"

namespace EightBit {
	struct MemoryMapping {

		enum AccessLevel { Unknown, ReadWrite, ReadOnly };

		Memory& memory;
		uint16_t begin = 0xffff;
		AccessLevel access = Unknown;
	};
}
