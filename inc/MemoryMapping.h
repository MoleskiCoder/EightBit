#pragma once

#include <cstdint>

#include "Memory.h"

namespace EightBit {
	struct MemoryMapping {

		enum AccessLevel { Unknown, ReadOnly, ReadWrite, };

		Memory& memory;
		uint16_t begin = 0xffff;
		AccessLevel access = Unknown;
	};
}
