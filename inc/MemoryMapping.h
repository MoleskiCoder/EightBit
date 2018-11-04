#pragma once

#include <cstdint>

namespace EightBit {

	class Memory;

	struct MemoryMapping {

		enum AccessLevel { Unknown, ReadOnly, ReadWrite, };

		Memory& memory;
		uint16_t begin = 0xffff;
		uint16_t mask = 0U;
		AccessLevel access = Unknown;
	};
}
