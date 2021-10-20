#pragma once

#include <cstdint>
#include <vector>

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

#include "byte_t.h"

class ram_t final {
private:
    std::vector<byte_t> m_bytes;

public:
#ifdef USE_SIMDJSON_JSON
    ram_t(simdjson::dom::array input);
#endif

#ifdef USE_RAPIDJSON_JSON
    ram_t(const rapidjson::Value& input);
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

    void add(byte_t byte);

    [[nodiscard]] auto begin() const noexcept { return m_bytes.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_bytes.end(); }

    [[nodiscard]] auto size() const noexcept { return m_bytes.size(); }

    void clear() noexcept { m_bytes.clear(); }

    [[nodiscard]] auto& operator[](size_t idx) noexcept { return m_bytes[idx]; }
    [[nodiscard]] auto operator[](size_t idx) const noexcept { return m_bytes[idx]; }
};
