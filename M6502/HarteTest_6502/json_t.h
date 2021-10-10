#pragma once

#include <string>

#include <boost/json.hpp>

class json_t {
protected:
    static const boost::json::value& get_value(const boost::json::object& object, std::string key);

    static int64_t get_int64(const boost::json::value& value);
    static uint16_t get_uint16(const boost::json::value& value);
    static uint8_t get_uint8(const boost::json::value& value);

    static int64_t get_int64(const boost::json::object& object, std::string key);
    static uint16_t get_uint16(const boost::json::object& object, std::string key);
    static uint8_t get_uint8(const boost::json::object& object, std::string key);

    static const boost::json::array& get_array(const boost::json::value& value);
    static const boost::json::array& get_array(const boost::json::object& object, std::string key);

    static const boost::json::string& get_string(const boost::json::value& value);
    static const boost::json::string& get_string(const boost::json::object& object, std::string key);
};
