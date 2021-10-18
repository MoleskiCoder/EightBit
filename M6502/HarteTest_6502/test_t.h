#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <tuple>

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

#include "cycles_t.h"
#include "state_t.h"

class test_t final {
private:
    std::string m_name;
    state_t m_initial_state;
    state_t m_final_state;
    cycles_t m_cycles;

public:

#ifdef USE_SIMDJSON_JSON
    test_t(simdjson::dom::element serialised);
#endif

#ifdef USE_BOOST_JSON
    test_t(const boost::json::object& serialised);
    test_t(const boost::json::value& serialised);
#endif

#ifdef USE_NLOHMANN_JSON
    test_t(const nlohmann::json& serialised);
#endif

#ifdef USE_JSONCPP_JSON
    test_t(const Json::Value& serialised);
#endif

    [[nodiscard]] constexpr const auto& name() const noexcept { return m_name; }
    [[nodiscard]] constexpr const auto& initial_state() const noexcept { return m_initial_state; }
    [[nodiscard]] constexpr const auto& final_state() const noexcept { return m_final_state; }
    [[nodiscard]] constexpr const auto& cycles() const noexcept { return m_cycles; }
};
