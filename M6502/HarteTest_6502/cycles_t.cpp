#include "stdafx.h"
#include "cycles_t.h"

cycles_t::cycles_t(size_t reserved) {
    m_cycles.reserve(reserved);
}

void cycles_t::add(const cycle_t& cycle) {
    m_cycles.push_back(cycle);
}

#ifdef USE_SIMDJSON_JSON

cycles_t::cycles_t(simdjson::dom::array input) {
    m_cycles.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif

#ifdef USE_BOOST_JSON

cycles_t::cycles_t(const boost::json::value& input)
: cycles_t(input.as_array()) {}

cycles_t::cycles_t(const boost::json::array& input) {
    m_cycles.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif

#ifdef USE_NLOHMANN_JSON

cycles_t::cycles_t(const nlohmann::json& input) {
    m_cycles.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif

#ifdef USE_JSONCPP_JSON

cycles_t::cycles_t(const Json::Value& input) {
    m_cycles.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif
