#pragma once

#include <string>

#include "simdjson/simdjson.h"

#include "cycles_t.h"
#include "state_t.h"

class test_t final {
private:
    std::string m_name;
    state_t m_initial_state;
    state_t m_final_state;
    cycles_t m_cycles;

public:
    test_t(simdjson::dom::element serialised);

    [[nodiscard]] constexpr const auto& name() const noexcept { return m_name; }
    [[nodiscard]] constexpr const auto& initial_state() const noexcept { return m_initial_state; }
    [[nodiscard]] constexpr const auto& final_state() const noexcept { return m_final_state; }
    [[nodiscard]] constexpr const auto& cycles() const noexcept { return m_cycles; }
};
