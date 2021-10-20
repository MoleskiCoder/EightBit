#pragma once

#include <string>

#include "simdjson/simdjson.h"

#include "cycles_t.h"
#include "state_t.h"

class test_t final {
private:
    simdjson::dom::element m_raw;

public:
    test_t(simdjson::dom::element input);

    [[nodiscard]] const auto name() const noexcept { return m_raw["name"]; }
    [[nodiscard]] const auto initial_state() const noexcept { return state_t(m_raw["initial"]); }
    [[nodiscard]] const auto final_state() const noexcept { return state_t(m_raw["final"]); }
    [[nodiscard]] const auto cycles() const noexcept { return cycles_t(m_raw["cycles"].get_array()); }
};
