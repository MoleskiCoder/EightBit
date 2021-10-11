#include "stdafx.h"
#include "state_t.h"
#include <cassert>

state_t::state_t() {}

#ifdef USE_BOOST_JSON

state_t::state_t(const boost::json::object& serialised) {
    initialise(serialised);
    assert(initialised());
}

state_t::state_t(const boost::json::value& serialised) {
    assert(serialised.is_object());
    initialise(serialised.get_object());
    assert(initialised());
}

void state_t::initialise(const boost::json::object& serialised) {

    assert(!initialised());

    m_pc = get_uint16(serialised, "pc");
    m_s = get_uint8(serialised, "s");
    m_a = get_uint8(serialised, "a");
    m_x = get_uint8(serialised, "x");
    m_y = get_uint8(serialised, "y");
    m_p = get_uint8(serialised, "p");

    const auto& ram_entries = get_array(serialised, "ram");
    for (const auto& ram_entry : ram_entries) {
        assert(ram_entry.is_array());
        const auto& ram_entry_array = ram_entry.as_array();
        assert(ram_entry_array.size() == 2);
        const auto address = get_uint16(ram_entry_array[0]);
        const auto value = get_uint8(ram_entry_array[1]);
        m_ram[address] = value;
    }

    m_initialised = true;
}

#endif

#ifdef USE_NLOHMANN_JSON

state_t::state_t(const nlohmann::json& serialised) {
    assert(serialised.is_object());
    initialise(serialised);
    assert(initialised());
}

void state_t::initialise(const nlohmann::json& serialised) {

    assert(!initialised());

    m_pc = serialised["pc"].get<uint16_t>();
    m_s = serialised["s"].get<uint8_t>();
    m_a = serialised["a"].get<uint8_t>();
    m_x = serialised["x"].get<uint8_t>();
    m_y = serialised["y"].get<uint8_t>();
    m_p = serialised["p"].get<uint8_t>();

    const auto& ram_entries = serialised["ram"];
    assert(ram_entries.is_array());
    for (const auto& ram_entry : ram_entries) {
        assert(ram_entry.is_array());
        assert(ram_entry.size() == 2);
        const auto address = ram_entry[0].get<uint16_t>();
        const auto value = ram_entry[1].get<uint8_t>();
        m_ram[address] = value;
    }

    m_initialised = true;
}

#endif

#ifdef USE_JSONCPP_JSON

state_t::state_t(const Json::Value& serialised) {
    assert(serialised.isObject());
    initialise(serialised);
    assert(initialised());
}

void state_t::initialise(const Json::Value& serialised) {

    assert(!initialised());

    m_pc = serialised["pc"].asUInt();
    m_s = serialised["s"].asUInt();
    m_a = serialised["a"].asUInt();
    m_x = serialised["x"].asUInt();
    m_y = serialised["y"].asUInt();
    m_p = serialised["p"].asUInt();

    const auto& ram_entries = serialised["ram"];
    assert(ram_entries.isArray());
    for (const auto& ram_entry : ram_entries) {
        assert(ram_entry.isArray());
        assert(ram_entry.size() == 2);
        const auto address = ram_entry[0].asUInt();
        const auto value = ram_entry[1].asUInt();
        m_ram[address] = value;
    }

    m_initialised = true;
}

#endif
