#pragma once

#include <cstdint>

#include "MemoryMapping.h"

namespace EightBit {
	class Mapper {
	public:
		[[nodiscard]] virtual MemoryMapping mapping(uint16_t address) noexcept = 0;
	};
}
