#pragma once

#include <cstdint>
#include <string>

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

class cycle_t final {
public:
	enum class action_t { read, write, unknown };

private:
	uint16_t m_address = 0xffff;
	uint8_t m_value = 0xff;
	action_t m_action = action_t::unknown;

public:
	[[nodiscard]] static action_t to_action(std::string value);
	[[nodiscard]] static std::string to_string(action_t value);

	cycle_t(uint16_t address, uint8_t value, action_t action);
	cycle_t(uint16_t address, uint8_t value, std::string action);

#ifdef USE_SIMDJSON_JSON
	cycle_t(simdjson::dom::element input);
	cycle_t(simdjson::dom::array input);
#endif

#ifdef USE_BOOST_JSON
	cycle_t(const boost::json::value& input);
	cycle_t(const boost::json::array& input);
#endif

#ifdef USE_NLOHMANN_JSON
	cycle_t(const nlohmann::json& input);
#endif

#ifdef USE_JSONCPP_JSON
	cycle_t(const Json::Value& input);
#endif

	[[nodiscard]] constexpr auto address() const noexcept { return m_address; }
	[[nodiscard]] constexpr auto value() const noexcept { return m_value; }
	[[nodiscard]] constexpr auto action() const noexcept { return m_action; }
};
