#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <tuple>

#ifdef USE_BOOST_JSON
#	include <boost/json.hpp>
#endif

#ifdef USE_NLOHMANN_JSON
#   include "nlohmann/json.hpp"
#endif

#ifdef USE_JSONCPP_JSON
#	include <json/json.h>
#endif

#ifdef USE_SIMDJSON_JSON
#	include "simdjson/simdjson.h"
#endif

#include "state_t.h"

class test_t final {
public:
    enum class action { read, write };
    
    typedef std::tuple<uint16_t, uint8_t, action> event_t;  // address, contents, action
    typedef std::vector<event_t> events_t;

    [[nodiscard]] static action to_action(std::string value);
    [[nodiscard]] static std::string to_string(action value);

private:
    std::string m_name;
    state_t m_initial_state;
    state_t m_final_state;
    events_t m_cycles; 

#ifdef USE_BOOST_JSON
    void initialise(const boost::json::object& serialised);
#endif

#ifdef USE_NLOHMANN_JSON
    void initialise(const nlohmann::json& serialised);
#endif

#ifdef USE_JSONCPP_JSON
    void initialise(const Json::Value& serialised);
#endif

#ifdef USE_SIMDJSON_JSON
    void initialise(simdjson::dom::element serialised);
#endif

public:

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

#ifdef USE_SIMDJSON_JSON
    test_t(simdjson::dom::element serialised);
#endif

    [[nodiscard]] constexpr const auto& name() const noexcept { return m_name; }
    [[nodiscard]] constexpr const auto& initial_state() const noexcept { return m_initial_state; }
    [[nodiscard]] constexpr const auto& final_state() const noexcept { return m_final_state; }
    [[nodiscard]] constexpr const auto& cycles() const noexcept { return m_cycles; }
};