#pragma once

#include <cstdint>

#include "MemoryMapping.h"

namespace EightBit {
	class Mapper {
	public:
		virtual ~Mapper() = default;

		[[nodiscard]] virtual MemoryMapping mapping(uint16_t address) = 0;
	};
}
