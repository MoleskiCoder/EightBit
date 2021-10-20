#pragma once

#include <cstdint>
#include <string>

#include "simdjson/simdjson.h"

class cycle_t final {
private:
	const uint16_t m_address = 0xffff;
	const uint8_t m_value = 0xff;
	const std::string m_action;

public:
	cycle_t(uint16_t address, uint8_t value, std::string action) noexcept;

	cycle_t(simdjson::dom::element input) noexcept;
	cycle_t(simdjson::dom::array input) noexcept;

	[[nodiscard]] constexpr auto address() const noexcept { return m_address; }
	[[nodiscard]] constexpr auto value() const noexcept { return m_value; }
	[[nodiscard]] auto action() const noexcept { return m_action; }
};
