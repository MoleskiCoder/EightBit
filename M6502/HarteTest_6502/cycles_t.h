#pragma once

#include <vector>

#ifdef USE_SIMDJSON_JSON
#	include "simdjson/simdjson.h"
#endif

#ifdef USE_RAPIDJSON_JSON
#	include "rapidjson/rapidjson.h"
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

#include "cycle_t.h"

class cycles_t final {
private:
	std::vector<cycle_t> m_cycles;

public:
	cycles_t(size_t reserved = 10);

#ifdef USE_SIMDJSON_JSON
	cycles_t(simdjson::dom::array input);
#endif

#ifdef USE_RAPIDJSON_JSON
	cycles_t(const rapidjson::Value& serialised);
#endif

#ifdef USE_BOOST_JSON
	cycles_t(const boost::json::value& input);
	cycles_t(const boost::json::array& input);
#endif

#ifdef USE_NLOHMANN_JSON
	cycles_t(const nlohmann::json& input);
#endif

#ifdef USE_JSONCPP_JSON
	cycles_t(const Json::Value& input);
#endif

	void add(const cycle_t& cycle);

	[[nodiscard]] auto begin() const { return m_cycles.begin(); }
	[[nodiscard]] auto end() const { return m_cycles.end(); }

	[[nodiscard]] auto size() const noexcept { return m_cycles.size(); }

	void clear() { m_cycles.clear(); }

	[[nodiscard]] auto& operator[](size_t idx) noexcept { return m_cycles[idx]; }
	[[nodiscard]] const auto& operator[](size_t idx) const noexcept { return m_cycles[idx]; }
};
