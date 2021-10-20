#include "stdafx.h"
#include "cycle_t.h"

cycle_t::cycle_t(uint16_t address, uint8_t value, std::string action) noexcept
: m_address(address),
  m_value(value),
  m_action(action) {}

cycle_t::cycle_t(simdjson::dom::element input) noexcept
: cycle_t(input.get_array()) {}

cycle_t::cycle_t(simdjson::dom::array input) noexcept
: m_address((uint16_t)(int64_t)input.at(0)),
  m_value((uint8_t)(int64_t)input.at(1)),
  m_action((std::string)input.at(2)) {}
