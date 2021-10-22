#pragma once

#include <vector>

#include "simdjson/simdjson.h"

#include "cycle_t.h"

class cycles_t final {
private:
	std::vector<cycle_t> m_cycles;

public:
	cycles_t(size_t reserved = 10);
	cycles_t(simdjson::dom::array input);

	void add(const cycle_t& cycle);

	[[nodiscard]] auto begin() const noexcept { return m_cycles.begin(); }
	[[nodiscard]] auto end() const noexcept { return m_cycles.end(); }

	[[nodiscard]] auto size() const noexcept { return m_cycles.size(); }

	[[nodiscard]] const auto& operator[](size_t idx) const noexcept { return m_cycles[idx]; }
};
