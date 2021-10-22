#pragma once

#include <cstdint>

#include "simdjson/simdjson.h"

class array_t {
private:
	const simdjson::dom::array m_raw;

protected:
	array_t(simdjson::dom::array input) noexcept;

	auto raw() const noexcept { return m_raw; }

	[[nodiscard]] auto at(size_t idx) const noexcept { return raw().at(idx); }
	[[nodiscard]] auto integer_at(size_t idx) const noexcept { return (int64_t)at(idx); }
	[[nodiscard]] auto address_at(size_t idx) const noexcept { return (uint16_t)integer_at(idx); }
	[[nodiscard]] auto byte_at(size_t idx) const noexcept { return (uint8_t)integer_at(idx); }

public:
	[[nodiscard]] auto begin() const noexcept { return m_raw.begin(); }
	[[nodiscard]] auto end() const noexcept { return m_raw.end(); }
	[[nodiscard]] auto size() const noexcept { return m_raw.size(); }
	[[nodiscard]] auto operator[](size_t idx) const noexcept { return m_raw.at(idx); }
};
