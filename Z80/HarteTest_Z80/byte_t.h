#pragma once

#include <cstdint>

#include "simdjson/simdjson.h"

#include "array_t.h"

class byte_t : public array_t {
public:
	byte_t(const simdjson::dom::array input) noexcept
	: array_t(input) {}

	[[nodiscard]] auto address() const noexcept { return address_at(0); }
	[[nodiscard]] auto value() const noexcept { return byte_at(1); }
};
