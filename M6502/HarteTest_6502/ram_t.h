#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#ifdef USE_SIMDJSON_JSON
#	include "simdjson/simdjson.h"
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

class ram_t {
private:
    std::vector<std::pair<uint16_t, uint8_t>> m_bytes;

public:
#ifdef USE_SIMDJSON_JSON
    ram_t(simdjson::dom::array input);
#endif

#ifdef USE_BOOST_JSON
    ram_t(const boost::json::value& input);
    ram_t(const boost::json::array& input);
#endif

#ifdef USE_NLOHMANN_JSON
    ram_t(const nlohmann::json& input);
#endif

#ifdef USE_JSONCPP_JSON
    ram_t(const Json::Value& input);
#endif

    [[nodiscard]] auto begin() const { return m_bytes.begin(); }
    [[nodiscard]] auto end() const { return m_bytes.end(); }

    [[nodiscard]] auto size() const noexcept { return m_bytes.size(); }

    void clear() { m_bytes.clear(); }

    [[nodiscard]] auto& at(size_t idx) { return m_bytes.at(idx); }
    [[nodiscard]] const auto& at(size_t idx) const { return m_bytes.at(idx); }
};
