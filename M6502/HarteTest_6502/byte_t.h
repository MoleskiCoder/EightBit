#pragma once

#include "simdjson/simdjson.h"

#include "array_t.h"

class byte_t : public array_t {
protected:
	[[nodiscard]] auto address_at(size_t idx) const noexcept { return (uint16_t)integer_at(idx); }
	[[nodiscard]] auto byte_at(size_t idx) const noexcept { return (uint8_t)integer_at(idx); }

public:
	byte_t(simdjson::dom::array input) noexcept;

	[[nodiscard]] auto address() const noexcept { return address_at(0); }
	[[nodiscard]] auto value() const noexcept { return byte_at(1); }
};
