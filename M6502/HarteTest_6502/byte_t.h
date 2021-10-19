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

class byte_t final {
private:
	uint16_t m_address = 0xffff;
	uint8_t m_value = 0xff;

public:
	byte_t(uint16_t address, uint8_t value) noexcept;

#ifdef USE_SIMDJSON_JSON
	byte_t(simdjson::dom::element input) noexcept;
	byte_t(simdjson::dom::array input) noexcept;
#endif

#ifdef USE_RAPIDJSON_JSON
	byte_t(const rapidjson::Value& input);
#endif

#ifdef USE_BOOST_JSON
	byte_t(const boost::json::value& input) noexcept;
	byte_t(const boost::json::array& input) noexcept;
#endif

#ifdef USE_NLOHMANN_JSON
	byte_t(const nlohmann::json& input) noexcept;
#endif

#ifdef USE_JSONCPP_JSON
	byte_t(const Json::Value& input);
#endif

	[[nodiscard]] constexpr auto address() const noexcept { return m_address; }
	[[nodiscard]] constexpr auto value() const noexcept { return m_value; }
};

