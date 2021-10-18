#include "stdafx.h"
#include "cycle_t.h"

#include <cassert>
#include <stdexcept>

cycle_t::action_t cycle_t::to_action(std::string value) {
    if (value == "read")
        return action_t::read;
    if (value == "write")
        return action_t::write;
    throw new std::out_of_range("Unknown action");
}

std::string cycle_t::to_string(action_t value) {
    if (value == action_t::read)
        return "read";
    if (value == action_t::write)
        return "write";
    throw new std::out_of_range("Unknown action");
}

cycle_t::cycle_t(uint16_t address, uint8_t value, action_t action)
: m_address(address),
  m_value(value),
  m_action(action) {}

cycle_t::cycle_t(uint16_t address, uint8_t value, std::string action)
: m_address(address),
  m_value(value),
  m_action(to_action(action)) {}

#ifdef USE_SIMDJSON_JSON

cycle_t::cycle_t(simdjson::dom::element input)
: cycle_t(input.get_array()) {}

cycle_t::cycle_t(simdjson::dom::array input)
: m_address((uint16_t)(uint64_t)input.at(0)),
  m_value((uint8_t)(uint64_t)input.at(1)),
  m_action(to_action((std::string)input.at(2))) {
    assert(input.size() == 3);
}

#endif

#ifdef USE_BOOST_JSON

cycle_t::cycle_t(const boost::json::value& input)
: cycle_t(input.as_array()) {}

cycle_t::cycle_t(const boost::json::array& input)
: m_address((uint16_t)input.at(0).as_int64()),
  m_value((uint8_t)input.at(1).as_int64()),
  m_action(to_action((std::string)input.at(2).as_string())) {
    assert(input.size() == 3);
};

#endif

#ifdef USE_NLOHMANN_JSON

cycle_t::cycle_t(const nlohmann::json& input)
: m_address(input[0].get<uint16_t>()),
  m_value(input[1].get<uint8_t>()),
  m_action(to_action(input[2].get<std::string>())) {
    assert(input.size() == 3);
}

#endif

#ifdef USE_JSONCPP_JSON
//cycle_t(const Json::Value& input);

cycle_t::cycle_t(const Json::Value& input)
    : m_address(input[0].asUInt()),
    m_value(input[1].asUInt()),
    m_action(to_action(input[2].asString())) {
    assert(input.size() == 3);
}


//for (const auto& cycles_entry : cycles_array) {
//    assert(cycles_entry.size() == 3);
//    const auto address = cycles_entry[0].asUInt();
//    const auto contents = cycles_entry[1].asUInt();
//    const auto action = to_action(cycles_entry[2].asString());
//    m_cycles.push_back({ address, contents, action });
//}

#endif
