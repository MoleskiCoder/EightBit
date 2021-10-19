#pragma once

#include <cstdint>

#ifdef USE_SIMDJSON_JSON
#	include "simdjson/simdjson.h"
#endif

#ifdef USE_RAPIDJSON_JSON
#   include "rapidjson/document.h"
#endif

#ifdef USE_BOOST_JSON
#	include <boost/json.hpp>
#endif

#ifdef USE_NLOHMANN_JSON
#   include "nlohmann/json.hpp"
#endif

#ifdef USE_JSONCPP_JSON
#	include <json/json.h>
#endif

#include "ram_t.h"

class state_t final {
private:
    uint16_t m_pc = 0xffff;
    uint8_t m_s = 0xff;
    uint8_t m_a = 0xff;
    uint8_t m_x = 0xff;
    uint8_t m_y = 0xff;
    uint8_t m_p = 0xff;
    ram_t m_ram;

public:
#ifdef USE_SIMDJSON_JSON
    state_t(simdjson::dom::element serialised);
#endif

#ifdef USE_RAPIDJSON_JSON
    state_t(const rapidjson::Value& serialised);
#endif

#ifdef USE_BOOST_JSON
    state_t(const boost::json::object& serialised);
    state_t(const boost::json::value& serialised);
#endif

#ifdef USE_NLOHMANN_JSON
    state_t(const nlohmann::json& serialised);
#endif

#ifdef USE_JSONCPP_JSON
    state_t(const Json::Value& serialised);
#endif

    [[nodiscard]] constexpr auto pc() const noexcept { return m_pc; }
    [[nodiscard]] constexpr auto s() const noexcept { return m_s; }
    [[nodiscard]] constexpr auto a() const noexcept { return m_a; }
    [[nodiscard]] constexpr auto x() const noexcept { return m_x; }
    [[nodiscard]] constexpr auto y() const noexcept { return m_y; }
    [[nodiscard]] constexpr auto p() const noexcept { return m_p; }
    [[nodiscard]] constexpr const auto& ram() const noexcept { return m_ram; }
};
