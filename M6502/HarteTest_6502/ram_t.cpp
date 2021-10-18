#include "stdafx.h"
#include "ram_t.h"

#ifdef USE_SIMDJSON_JSON

ram_t::ram_t(simdjson::dom::array input) {
    m_bytes.reserve(input.size());
    for (const auto byte : input) {
        assert(byte.is_array());
        const auto ram_entry_array = byte.get_array();
        assert(byte.size() == 2);
        auto iterator = byte.begin();
        const auto address = (uint16_t)(uint64_t)*iterator;
        const auto value = (uint8_t)(uint64_t)*++iterator;
        m_bytes.push_back({ address, value });
    }
}

#endif

#ifdef USE_BOOST_JSON

ram_t::ram_t(const boost::json::array& input) {
    m_bytes.reserve(input.size());
    for (const auto& byte : input) {
        const auto& ram_entry_array = byte.get_array();
        assert(ram_entry_array.size() == 2);
        const auto address = (uint16_t)ram_entry_array[0].get_int64();
        const auto value = (uint8_t)ram_entry_array[1].get_int64();
        m_bytes.push_back({ address, value });
    }
}

ram_t::ram_t(const boost::json::value& input)
: ram_t(input.get_array()) {}

#endif

#ifdef USE_NLOHMANN_JSON

ram_t::ram_t(const nlohmann::json& input) {
    assert(input.is_array());
    m_bytes.reserve(input.size());
    for (const auto& byte : input) {
        assert(byte.is_array());
        assert(byte.size() == 2);
        const auto address = byte[0].get<uint16_t>();
        const auto value = byte[1].get<uint8_t>();
        m_bytes.push_back({ address, value });
    }
}

#endif

#ifdef USE_JSONCPP_JSON

ram_t::ram_t(const Json::Value& input) {
    assert(input.isArray());
    m_bytes.reserve(input.size());
    for (const auto& byte : input) {
        assert(byte.isArray());
        assert(byte.size() == 2);
        const auto address = byte[0].asUInt();
        const auto value = byte[1].asUInt();
        m_bytes.push_back({ address, value });
    }
}

#endif
