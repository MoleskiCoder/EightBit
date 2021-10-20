#include "stdafx.h"
#include "byte_t.h"

#include <cassert>

#ifdef USE_SIMDJSON_JSON

byte_t::byte_t(simdjson::dom::element input) noexcept
: byte_t(input.get_array()) {}

#ifdef JSON_ACCESS_DATA_VIA_OBJECT

byte_t::byte_t(simdjson::dom::array input) noexcept
: m_raw(input) {}

#else // JSON_ACCESS_DATA_VIA_OBJECT

#ifdef JSON_PREFER_ITERATOR_TO_INDEX

byte_t::byte_t(simdjson::dom::array input) noexcept {
    assert(input.size() == 2);
    auto iterator = input.begin();
    m_address = (uint16_t)(int64_t)*iterator;
    m_value = (uint8_t)(int64_t)*++iterator;
}

#else // JSON_PREFER_ITERATOR_TO_INDEX

byte_t::byte_t(simdjson::dom::array input) noexcept
: m_address((uint16_t)(int64_t)input.at(0)),
  m_value((uint8_t)(int64_t)input.at(1)) {
    assert(input.size() == 2);
}

#endif // JSON_PREFER_ITERATOR_TO_INDEX

#endif // JSON_ACCESS_DATA_VIA_OBJECT

#endif // USE_SIMDJSON_JSON

#ifdef USE_RAPIDJSON_JSON

#ifdef JSON_ACCESS_DATA_VIA_OBJECT

byte_t::byte_t(const rapidjson::Value& input)
: m_raw(input) {}

#else

byte_t::byte_t(const rapidjson::Value& input)
: m_address((uint16_t)input[0].GetInt64()),
  m_value((uint8_t)input[1].GetInt64()) {}

#endif

#endif

#ifdef USE_BOOST_JSON

byte_t::byte_t(const boost::json::value& input) noexcept
: byte_t(input.get_array()) {}


#ifdef JSON_ACCESS_DATA_VIA_OBJECT

byte_t::byte_t(const boost::json::array& input) noexcept
: m_raw(input) {}

#else

byte_t::byte_t(const boost::json::array& input) noexcept
: m_address((uint16_t)input[0].get_int64()),
  m_value((uint8_t)input[1].get_int64()) {
    assert(input.size() == 2);
}

#endif

#endif

#ifdef USE_NLOHMANN_JSON

#ifdef JSON_ACCESS_DATA_VIA_OBJECT

byte_t::byte_t(const nlohmann::json& input) noexcept
: m_raw(input) {}

#else

byte_t::byte_t(const nlohmann::json& input) noexcept
: m_address(input[0].get<uint16_t>()),
  m_value(input[1].get<uint8_t>()) {
    assert(input.size() == 2);
}

#endif


#endif

#ifdef USE_JSONCPP_JSON

#ifdef JSON_ACCESS_DATA_VIA_OBJECT

byte_t::byte_t(const Json::Value& input)
: m_raw(input) {}

#else

byte_t::byte_t(const Json::Value& input)
: m_address((uint16_t)input[0].asInt64()),
  m_value((uint8_t)input[1].asInt64()) {
    assert(input.size() == 2);
}

#endif

#endif
