#pragma once

#include "simdjson/simdjson.h"

class array_t {
private:
	const simdjson::dom::array m_raw;

protected:
	[[nodiscard]] auto raw() const noexcept { return m_raw; }

public:
	array_t(simdjson::dom::array input) noexcept;

	[[nodiscard]] auto begin() const noexcept { return m_raw.begin(); }
	[[nodiscard]] auto end() const noexcept { return m_raw.end(); }
	[[nodiscard]] auto size() const noexcept { return m_raw.size(); }

	[[nodiscard]] auto at(size_t idx) const noexcept { return raw().at(idx); }
	[[nodiscard]] auto operator[](size_t idx) const noexcept { return at(idx); }

	[[nodiscard]] auto integer_at(size_t idx) const noexcept { return at(idx).get_int64(); }
};