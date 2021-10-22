#pragma once

#include "simdjson/simdjson.h"

#include "array_t.h"

class byte_t : public array_t {
public:
	byte_t(simdjson::dom::array input) noexcept;

	[[nodiscard]] auto address() const noexcept { return address_at(0); }
	[[nodiscard]] auto value() const noexcept { return byte_at(1); }
};
