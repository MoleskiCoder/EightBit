#pragma once

#include <string_view>

#include "simdjson/simdjson.h"

#include "byte_t.h"

class cycle_t final : public array_t {
public:
	cycle_t(const simdjson::dom::array input) noexcept
	: array_t(input) {}

	[[nodiscard]] auto address() const noexcept { return address_at(0); }
	[[nodiscard]] auto value() const noexcept { return maybe_byte_at(1); }
	[[nodiscard]] auto action() const noexcept { return string_at(2); }
};
