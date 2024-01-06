#pragma once

#include <string_view>

#include "simdjson/simdjson.h"

#include "byte_t.h"

class cycle_t final : public byte_t {
public:
	cycle_t(simdjson::dom::array input) noexcept;

	[[nodiscard]] auto action() const noexcept { return std::string(std::string_view(at(2))); }
};
