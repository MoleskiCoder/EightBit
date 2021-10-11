#include "stdafx.h"
#include "json_t.h"

#ifdef USE_BOOST_JSON

#include <cassert>

const boost::json::value& json_t::get_value(const boost::json::object& object, std::string key) {
    auto* value = object.if_contains(key);
    assert(value != nullptr);
    return *value;
}

int64_t json_t::get_int64(const boost::json::value& value) {
    assert(value.is_number());
    assert(value.is_int64());
    return value.get_int64();
}

uint16_t json_t::get_uint16(const boost::json::value& value) {
    return static_cast<uint16_t>(get_int64(value));
}

uint8_t json_t::get_uint8(const boost::json::value& value) {
    return static_cast<uint8_t>(get_int64(value));
}

int64_t json_t::get_int64(const boost::json::object& object, std::string key) {
    return get_int64(get_value(object, key));
}

uint16_t json_t::get_uint16(const boost::json::object& object, std::string key) {
    return static_cast<uint16_t>(get_int64(object, key));
}

uint8_t json_t::get_uint8(const boost::json::object& object, std::string key) {
    return static_cast<uint8_t>(get_int64(object, key));
}

const boost::json::array& json_t::get_array(const boost::json::value& value) {
    assert(value.is_array());
    return value.get_array();
}

const boost::json::array& json_t::get_array(const boost::json::object& object, std::string key) {
    return get_array(get_value(object, key));
}

const boost::json::string& json_t::get_string(const boost::json::value& value) {
    assert(value.is_string());
    return value.get_string();
}

const boost::json::string& json_t::get_string(const boost::json::object& object, std::string key) {
    return get_string(get_value(object, key));
}

#endif