#include "stdafx.h"
#include "cycles_t.h"

cycles_t::cycles_t(size_t reserved) {
    m_cycles.reserve(reserved);
}

void cycles_t::add(const cycle_t& cycle) {
    assert(m_cycles.capacity() >= (m_cycles.size() + 1));
    m_cycles.push_back(cycle);
}

#ifdef USE_SIMDJSON_JSON

cycles_t::cycles_t(simdjson::dom::array input) {
    assert(m_cycles.empty());
    m_cycles.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif

#ifdef USE_RAPIDJSON_JSON

cycles_t::cycles_t(const rapidjson::Value& input) {
    assert(m_cycles.empty());
    m_cycles.reserve(input.Size());
    for (const auto& entry : input.GetArray())
        add(entry);
}

#endif

#ifdef USE_BOOST_JSON

cycles_t::cycles_t(const boost::json::value& input)
: cycles_t(input.get_array()) {}

cycles_t::cycles_t(const boost::json::array& input) {
    assert(m_cycles.empty());
    m_cycles.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif

#ifdef USE_NLOHMANN_JSON

cycles_t::cycles_t(const nlohmann::json& input) {
    assert(m_cycles.empty());
    m_cycles.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif

#ifdef USE_JSONCPP_JSON

cycles_t::cycles_t(const Json::Value& input) {
    assert(m_cycles.empty());
    m_cycles.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif
