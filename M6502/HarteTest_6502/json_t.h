#pragma once

#include <cstdint>
#include <string>

#include <boost/json.hpp>

class json_t {
protected:
    [[nodiscard]] static const boost::json::value& get_value(const boost::json::object& object, std::string key);

    [[nodiscard]] static int64_t get_int64(const boost::json::value& value);
    [[nodiscard]] static uint16_t get_uint16(const boost::json::value& value);
    [[nodiscard]] static uint8_t get_uint8(const boost::json::value& value);

    [[nodiscard]] static int64_t get_int64(const boost::json::object& object, std::string key);
    [[nodiscard]] static uint16_t get_uint16(const boost::json::object& object, std::string key);
    [[nodiscard]] static uint8_t get_uint8(const boost::json::object& object, std::string key);

    [[nodiscard]] static const boost::json::array& get_array(const boost::json::value& value);
    [[nodiscard]] static const boost::json::array& get_array(const boost::json::object& object, std::string key);

    [[nodiscard]] static const boost::json::string& get_string(const boost::json::value& value);
    [[nodiscard]] static const boost::json::string& get_string(const boost::json::object& object, std::string key);
};
