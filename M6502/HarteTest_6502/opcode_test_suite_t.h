#pragma once

#include <string>

#ifdef USE_BOOST_JSON
#	include <boost/json.hpp>
#endif

#ifdef USE_NLOHMANN_JSON
#   include "nlohmann/json.hpp"
#endif

#ifdef USE_JSONCPP_JSON
#   include <memory>
#	include <json/json.h>
#endif

#ifdef USE_SIMDJSON_JSON
#	include "simdjson/simdjson.h"
#endif

class opcode_test_suite_t final {
private:
#ifdef USE_JSONCPP_JSON
    static std::unique_ptr<Json::CharReader> m_reader;
#endif
#ifdef USE_SIMDJSON_JSON
    static std::unique_ptr<simdjson::dom::parser> m_parser;
#endif

    [[nodiscard]] static std::string read(std::string path);

    std::string m_path;
#ifdef USE_BOOST_JSON
    boost::json::value m_raw;
#endif
#ifdef USE_NLOHMANN_JSON
    nlohmann::json m_raw;
#endif
#ifdef USE_JSONCPP_JSON
    Json::Value m_raw;
#endif
#ifdef USE_SIMDJSON_JSON
    simdjson::dom::element m_raw;
#endif

public:
    opcode_test_suite_t(std::string path);

    [[nodiscard]] constexpr const auto& path() const noexcept { return m_path; }

#ifdef JSON_PREFER_PASS_BY_VALUE
    [[nodiscard]] const auto raw() const noexcept { return m_raw; }
#else
    [[nodiscard]] constexpr const auto& raw() const noexcept { return m_raw; }
#endif

#ifdef USE_BOOST_JSON
    [[nodiscard]] const boost::json::array& get_array() const noexcept;
#endif

    void load();
};
