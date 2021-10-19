#include "stdafx.h"
#include "ram_t.h"

void ram_t::add(const byte_t& byte) {
    assert(m_bytes.capacity() >= (m_bytes.size() + 1));
    m_bytes.push_back(byte);
}

#ifdef USE_SIMDJSON_JSON

ram_t::ram_t(simdjson::dom::array input) {
    assert(m_bytes.empty());
    m_bytes.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif

#ifdef USE_RAPIDJSON_JSON

ram_t::ram_t(const rapidjson::Value& input) {
    assert(m_bytes.empty());
    m_bytes.reserve(input.Size());
    for (const auto& entry : input.GetArray())
        add(entry);
}

#endif

#ifdef USE_BOOST_JSON

ram_t::ram_t(const boost::json::array& input) {
    assert(m_bytes.empty());
    m_bytes.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

ram_t::ram_t(const boost::json::value& input)
: ram_t(input.get_array()) {}

#endif

#ifdef USE_NLOHMANN_JSON

ram_t::ram_t(const nlohmann::json& input) {
    assert(m_bytes.empty());
    m_bytes.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif

#ifdef USE_JSONCPP_JSON

ram_t::ram_t(const Json::Value& input) {
    assert(m_bytes.empty());
    m_bytes.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}

#endif
