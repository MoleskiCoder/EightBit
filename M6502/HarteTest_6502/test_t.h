#pragma once

#include <string>

#include "simdjson/simdjson.h"

#include "cycles_t.h"
#include "state_t.h"

class test_t final {
private:
    simdjson::dom::element m_raw;

    [[nodiscard]] auto at(std::string key) const noexcept { return m_raw[key]; }
    [[nodiscard]] auto array_at(std::string key) const noexcept { return at(key).get_array(); }

public:
    test_t(simdjson::dom::element input) noexcept;

    [[nodiscard]] auto name() const noexcept { return at("name"); }
    [[nodiscard]] auto initial_state() const noexcept { return state_t(at("initial")); }
    [[nodiscard]] auto final_state() const noexcept { return state_t(at("final")); }
    [[nodiscard]] auto cycles() const noexcept { return cycles_t(array_at("cycles")); }
};
