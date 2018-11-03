#pragma once

#include <cstdint>

namespace EightBit {

	class MemoryInterface;

	struct MemoryMapping {

		enum AccessLevel { Unknown, ReadOnly, ReadWrite, };

		MemoryInterface& memory;
		uint16_t begin = 0xffff;
		uint16_t mask = 0U;
		AccessLevel access = Unknown;
	};
}
