#include "stdafx.h"
#include "state_t.h"
#include <cassert>

state_t::state_t() {}

#ifdef USE_SIMDJSON_JSON

state_t::state_t(const simdjson::dom::element serialised)
: m_pc((uint16_t)(uint64_t)serialised["pc"]),
  m_s((uint8_t)(uint64_t)serialised["s"]),
  m_a((uint8_t)(uint64_t)serialised["a"]),
  m_x((uint8_t)(uint64_t)serialised["x"]),
  m_y((uint8_t)(uint64_t)serialised["y"]),
  m_p((uint8_t)(uint64_t)serialised["p"]) {

    const auto ram_entries = serialised["ram"].get_array();
    m_ram.reserve(ram_entries.size());
    for (const auto ram_entry : ram_entries) {
        assert(ram_entry.is_array());
        const auto ram_entry_array = ram_entry.get_array();
        assert(ram_entry_array.size() == 2);
        const auto address = (uint16_t)(uint64_t)ram_entry_array.at(0);
        const auto value = (uint8_t)(uint64_t)ram_entry_array.at(1);
        m_ram.push_back({ address, value });
    }
}

#endif

#ifdef USE_BOOST_JSON

state_t::state_t(const boost::json::object& serialised)
: m_pc((uint16_t)serialised.at("pc").as_int64()),
  m_s((uint8_t)serialised.at("s").as_int64()),
  m_a((uint8_t)serialised.at("a").as_int64()),
  m_x((uint8_t)serialised.at("x").as_int64()),
  m_y((uint8_t)serialised.at("y").as_int64()),
  m_p((uint8_t)serialised.at("p").as_int64()) {

    const auto& ram_entries = serialised.at("ram").as_array();
    m_ram.reserve(ram_entries.size());
    for (const auto& ram_entry : ram_entries) {
        const auto& ram_entry_array = ram_entry.as_array();
        assert(ram_entry_array.size() == 2);
        const auto address = (uint16_t)ram_entry_array.at(0).as_int64();
        const auto value = (uint8_t)ram_entry_array.at(1).as_int64();
        m_ram.push_back({ address, value });
    }
}

state_t::state_t(const boost::json::value& serialised)
: state_t(serialised.as_object()) {}

#endif

#ifdef USE_NLOHMANN_JSON

state_t::state_t(const nlohmann::json& serialised)
: m_pc(serialised["pc"].get<uint16_t>()),
  m_s(serialised["s"].get<uint8_t>()),
  m_a(serialised["a"].get<uint8_t>()),
  m_x(serialised["x"].get<uint8_t>()),
  m_y(serialised["y"].get<uint8_t>()),
  m_p(serialised["p"].get<uint8_t>()) {

    const auto& ram_entries = serialised["ram"];
    assert(ram_entries.is_array());
    for (const auto& ram_entry : ram_entries) {
        assert(ram_entry.is_array());
        assert(ram_entry.size() == 2);
        const auto address = ram_entry[0].get<uint16_t>();
        const auto value = ram_entry[1].get<uint8_t>();
        m_ram.push_back({ address, value });
    }
}

#endif

#ifdef USE_JSONCPP_JSON

state_t::state_t(const Json::Value& serialised)
: m_pc(serialised["pc"].asUInt()),
  m_s(serialised["s"].asUInt()),
  m_a(serialised["a"].asUInt()),
  m_x(serialised["x"].asUInt()),
  m_y(serialised["y"].asUInt()),
  m_p(serialised["p"].asUInt()) {

    const auto& ram_entries = serialised["ram"];
    assert(ram_entries.isArray());
    for (const auto& ram_entry : ram_entries) {
        assert(ram_entry.isArray());
        assert(ram_entry.size() == 2);
        const auto address = ram_entry[0].asUInt();
        const auto value = ram_entry[1].asUInt();
        m_ram.push_back({ address, value });
    }
}

#endif
