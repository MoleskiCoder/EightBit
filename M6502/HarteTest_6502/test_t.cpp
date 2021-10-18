#include "stdafx.h"
#include "test_t.h"
#include <cassert>

#ifdef USE_SIMDJSON_JSON

test_t::test_t(const simdjson::dom::element serialised)
: m_name(serialised["name"]),
  m_initial_state(serialised["initial"]),
  m_final_state(serialised["final"]),
  m_cycles(serialised["cycles"].get_array()) {}

#endif

#ifdef USE_RAPIDJSON_JSON

test_t::test_t(const rapidjson::Value& serialised)
: m_name(serialised["name"].GetString()),
  m_initial_state(serialised["initial"]),
  m_final_state(serialised["final"]),
  m_cycles(serialised["cycles"]) {}

#endif

#ifdef USE_BOOST_JSON

test_t::test_t(const boost::json::object& serialised)
: m_name(serialised.at("name").get_string()),
  m_initial_state(serialised.at("initial")),
  m_final_state(serialised.at("final")),
  m_cycles(serialised.at("cycles")) {}

test_t::test_t(const boost::json::value& serialised)
: test_t(serialised.get_object()) {}

#endif

#ifdef USE_NLOHMANN_JSON

test_t::test_t(const nlohmann::json& serialised)
: m_name(serialised["name"].get<std::string>()),
  m_initial_state(serialised["initial"]),
  m_final_state(serialised["final"]),
  m_cycles(serialised["cycles"]) {}

#endif

#ifdef USE_JSONCPP_JSON

test_t::test_t(const Json::Value& serialised)
: m_name(serialised["name"].asString()),
  m_initial_state(serialised["initial"]),
  m_final_state(serialised["final"]),
  m_cycles(serialised["cycles"]) {}

#endif
