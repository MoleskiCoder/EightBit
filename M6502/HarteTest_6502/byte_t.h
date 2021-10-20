#pragma once

#include <cstdint>

#include "simdjson/simdjson.h"

class byte_t final {
private:
	const simdjson::dom::array m_raw;

public:
	byte_t(simdjson::dom::element input) noexcept;
	byte_t(simdjson::dom::array input) noexcept;

	[[nodiscard]] auto address() const noexcept { return (uint16_t)(int64_t)m_raw.at(0); }
	[[nodiscard]] auto value() const noexcept { return (uint8_t)(int64_t)m_raw.at(1); }
};
