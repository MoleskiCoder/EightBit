#include "stdafx.h"
#include "test_t.h"
#include <cassert>
#include <stdexcept>

test_t::action test_t::to_action(std::string value) {
    if (value == "read")
        return action::read;
    if (value == "write")
        return action::write;
    throw new std::out_of_range("Unknown action");
}

std::string test_t::to_string(action value) {
    if (value == action::read)
        return "read";
    if (value == action::write)
        return "write";
    throw new std::out_of_range("Unknown action");
}

#ifdef USE_BOOST_JSON

test_t::test_t(const boost::json::object& serialised) {
    initialise(serialised);
}

test_t::test_t(const boost::json::value& serialised) {
    initialise(serialised.as_object());
}

void test_t::initialise(const boost::json::object& serialised) {

    m_name = serialised.at("name").as_string();
    m_initial_state = state_t(serialised.at("initial"));
    m_final_state = state_t(serialised.at("final"));

    const auto& cycles_array = serialised.at("cycles").as_array();
    m_cycles.reserve(cycles_array.size());

    for (const auto& cycles_entry : cycles_array) {
        const auto& cycle_array = cycles_entry.as_array();
        assert(cycle_array.size() == 3);
        const auto address = (uint16_t)cycle_array[0].as_int64();
        const auto contents = (uint8_t)cycle_array[1].as_int64();
        const auto action = to_action((std::string)cycle_array[2].as_string());
        m_cycles.push_back({ address, contents, action });
    }
}

#endif

#ifdef USE_NLOHMANN_JSON

test_t::test_t(const nlohmann::json& serialised) {
    assert(serialised.is_object());
    initialise(serialised);
}

void test_t::initialise(const nlohmann::json& serialised) {

    m_name = serialised["name"].get<std::string>();
    m_initial_state = state_t(serialised["initial"]);
    m_final_state = state_t(serialised["final"]);

    const auto& cycles_array = serialised["cycles"];
    m_cycles.reserve(cycles_array.size());

    for (const auto& cycles_entry : cycles_array) {
        assert(cycles_entry.size() == 3);
        const auto address = cycles_entry[0].get<uint16_t>();
        const auto contents = cycles_entry[1].get<uint8_t>();
        const auto action = to_action(cycles_entry[2].get<std::string>());
        m_cycles.push_back({ address, contents, action });
    }
}

#endif

#ifdef USE_JSONCPP_JSON

test_t::test_t(const Json::Value& serialised) {
    assert(serialised.isObject());
    initialise(serialised);
}

void test_t::initialise(const Json::Value& serialised) {

    m_name = serialised["name"].asString();
    m_initial_state = state_t(serialised["initial"]);
    m_final_state = state_t(serialised["final"]);

    const auto& cycles_array = serialised["cycles"];
    m_cycles.reserve(cycles_array.size());

    for (const auto& cycles_entry : cycles_array) {
        assert(cycles_entry.size() == 3);
        const auto address = cycles_entry[0].asUInt();
        const auto contents = cycles_entry[1].asUInt();
        const auto action = to_action(cycles_entry[2].asString());
        m_cycles.push_back({ address, contents, action });
    }
}

#endif

#ifdef USE_SIMDJSON_JSON

test_t::test_t(const simdjson::dom::element serialised) {
    assert(serialised.is_object());
    initialise(serialised);
}

void test_t::initialise(const simdjson::dom::element serialised) {

    m_name = serialised["name"];
    m_initial_state = state_t(serialised["initial"]);
    m_final_state = state_t(serialised["final"]);

    const auto cycles_array = serialised["cycles"].get_array();
    m_cycles.reserve(cycles_array.size());

    for (const auto cycles_entry : cycles_array) {
        const auto cycle_array = cycles_entry.get_array();
        assert(cycle_array.size() == 3);
        const auto address = (uint16_t)(uint64_t)cycle_array.at(0);
        const auto contents = (uint8_t)(uint64_t)cycle_array.at(1);
        const auto action = to_action((std::string)cycle_array.at(2));
        m_cycles.push_back({ address, contents, action });
    }
}

#endif
