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

#ifdef JSON_ACCESS_DATA_VIA_OBJECT

#ifdef USE_SIMDJSON_JSON
	simdjson::dom::array m_raw;
#endif

#ifdef USE_RAPIDJSON_JSON
	const rapidjson::Value& m_raw;
#endif

#ifdef USE_BOOST_JSON
	const boost::json::array& m_raw;
#endif

#ifdef USE_NLOHMANN_JSON
	const nlohmann::json& m_raw;
#endif

#ifdef USE_JSONCPP_JSON
	const Json::Value& m_raw;
#endif

#else

	uint16_t m_address = 0xffff;
	uint8_t m_value = 0xff;

#endif

public:

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

#ifdef JSON_ACCESS_DATA_VIA_OBJECT
#ifdef USE_SIMDJSON_JSON
	[[nodiscard]] auto address() const noexcept { return (uint16_t)(int64_t)*m_raw.begin(); }
	[[nodiscard]] auto value() const noexcept { return (uint8_t)(int64_t)m_raw.at(1); }
#endif
#ifdef USE_RAPIDJSON_JSON
	[[nodiscard]] auto address() const noexcept { return (uint16_t)m_raw[0].GetInt64(); }
	[[nodiscard]] auto value() const noexcept { return (uint8_t)m_raw[1].GetInt64(); }
#endif
#ifdef USE_BOOST_JSON
	[[nodiscard]] auto address() const noexcept { return (uint16_t)m_raw[0].get_int64(); }
	[[nodiscard]] auto value() const noexcept { return (uint8_t)m_raw[1].get_int64(); }
#endif
#ifdef USE_NLOHMANN_JSON
	[[nodiscard]] auto address() const noexcept { return m_raw[0].get<uint16_t>(); }
	[[nodiscard]] auto value() const noexcept { return m_raw[1].get<uint8_t>(); }
#endif
#ifdef USE_JSONCPP_JSON
	[[nodiscard]] auto address() const noexcept { return (uint16_t)m_raw[0].asInt64(); }
	[[nodiscard]] auto value() const noexcept { return (uint8_t)m_raw[1].asInt64(); }
#endif
#else
	[[nodiscard]] constexpr auto address() const noexcept { return m_address; }
	[[nodiscard]] constexpr auto value() const noexcept { return m_value; }
#endif
};
