#include "stdafx.h"
#include "cycle_t.h"

#include <cassert>
#include <stdexcept>

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

#ifdef USE_SIMDJSON_JSON

cycle_t::cycle_t(simdjson::dom::element input) noexcept
: cycle_t(input.get_array()) {}

cycle_t::cycle_t(simdjson::dom::array input) noexcept
: m_iterator(input.begin()),
  m_address((uint16_t)(uint64_t)*m_iterator),
  m_value((uint8_t)(uint64_t)*++m_iterator),
  m_action(to_action((std::string)*++m_iterator)) {
    assert(input.size() == 3);
}

#endif

#ifdef USE_RAPIDJSON_JSON

cycle_t::cycle_t(const rapidjson::Value& input)
: m_address((uint16_t)input[0].GetInt64()),
  m_value((uint8_t)input[1].GetInt64()),
  m_action(to_action(input[2].GetString())) {
    assert(input.Size() == 3);
}

#endif

#ifdef USE_BOOST_JSON

cycle_t::cycle_t(const boost::json::value& input) noexcept
: cycle_t(input.get_array()) {}

cycle_t::cycle_t(const boost::json::array& input) noexcept
: m_address((uint16_t)input[0].get_int64()),
  m_value((uint8_t)input[1].get_int64()),
  m_action(to_action((std::string)input[2].get_string())) {
    assert(input.size() == 3);
};

#endif

#ifdef USE_NLOHMANN_JSON

cycle_t::cycle_t(const nlohmann::json& input) noexcept
: m_address(input[0].get<uint16_t>()),
  m_value(input[1].get<uint8_t>()),
  m_action(to_action(input[2].get<std::string>())) {
    assert(input.size() == 3);
}

#endif

#ifdef USE_JSONCPP_JSON

cycle_t::cycle_t(const Json::Value& input)
: m_address(input[0].asUInt()),
  m_value(input[1].asUInt()),
  m_action(to_action(input[2].asString())) {
    assert(input.size() == 3);
}

#endif
