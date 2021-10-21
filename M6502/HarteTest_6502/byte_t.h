#pragma once

#include <cstdint>

#include "simdjson/simdjson.h"

class byte_t final {
private:
	const simdjson::dom::array m_raw;

	[[nodiscard]] auto at(size_t idx) const noexcept { return m_raw.at(idx); }
	[[nodiscard]] auto integer_at(size_t idx) const noexcept { return (int64_t)at(idx); }
	[[nodiscard]] auto address_at(size_t idx) const noexcept { return (uint16_t)integer_at(idx); }
	[[nodiscard]] auto byte_at(size_t idx) const noexcept { return (uint8_t)integer_at(idx); }

public:
	byte_t(simdjson::dom::array input) noexcept;

	[[nodiscard]] auto address() const noexcept { return address_at(0); }
	[[nodiscard]] auto value() const noexcept { return byte_at(1); }
};
