#pragma once

#include <cstdint>

#include "MemoryMapping.h"

namespace EightBit {
	class Mapper {
	public:
		virtual ~Mapper() noexcept = default;
		[[nodiscard]] virtual const MemoryMapping& mapping(uint16_t address) noexcept = 0;
	};
}
