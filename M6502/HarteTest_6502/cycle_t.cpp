#include "stdafx.h"
#include "cycle_t.h"

#include <cassert>

cycle_t::action_t cycle_t::to_action(std::string value) noexcept {
    if (value == "read")
        return action_t::read;
    if (value == "write")
        return action_t::write;
    return action_t::unknown;
}

std::string cycle_t::to_string(action_t value) noexcept {
    if (value == action_t::read)
        return "read";
    if (value == action_t::write)
        return "write";
    return "unknown";
}

cycle_t::cycle_t(uint16_t address, uint8_t value, action_t action) noexcept
: m_address(address),
  m_value(value),
  m_action(action) {}

cycle_t::cycle_t(simdjson::dom::element input) noexcept
: cycle_t(input.get_array()) {}

cycle_t::cycle_t(simdjson::dom::array input) noexcept {
    assert(input.size() == 3);
    auto iterator = input.begin();
    m_address = (uint16_t)(int64_t)*iterator;
    m_value = (uint8_t)(int64_t)*++iterator;
    m_action = to_action((std::string)*++iterator);
}
