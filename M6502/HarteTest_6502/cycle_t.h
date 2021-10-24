#pragma once

#include <string>

#include "simdjson/simdjson.h"

#include "byte_t.h"

class cycle_t final : public byte_t {
public:
	cycle_t(simdjson::dom::array input) noexcept;

	[[nodiscard]] std::string_view action() const noexcept { return at(2).get_string(); }
};
