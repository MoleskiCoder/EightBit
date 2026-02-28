#pragma once

#include <cstdint>

#include "simdjson/simdjson.h"

#include "byte_t.h"

class port_t : public byte_t {
public:
	port_t(const simdjson::dom::array input) noexcept
	: byte_t(input) {}

	[[nodiscard]] auto type() const noexcept { return string_at(2); }
};
