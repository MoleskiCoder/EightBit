#include "stdafx.h"
#include "state_t.h"
#include <cassert>

#ifdef USE_SIMDJSON_JSON

state_t::state_t(const simdjson::dom::element serialised)
: m_pc((uint16_t)(uint64_t)serialised["pc"]),
  m_s((uint8_t)(uint64_t)serialised["s"]),
  m_a((uint8_t)(uint64_t)serialised["a"]),
  m_x((uint8_t)(uint64_t)serialised["x"]),
  m_y((uint8_t)(uint64_t)serialised["y"]),
  m_p((uint8_t)(uint64_t)serialised["p"]),
  m_ram(serialised["ram"].get_array()) {}

#endif

#ifdef USE_BOOST_JSON

state_t::state_t(const boost::json::object& serialised)
: m_pc((uint16_t)serialised.at("pc").get_int64()),
  m_s((uint8_t)serialised.at("s").get_int64()),
  m_a((uint8_t)serialised.at("a").get_int64()),
  m_x((uint8_t)serialised.at("x").get_int64()),
  m_y((uint8_t)serialised.at("y").get_int64()),
  m_p((uint8_t)serialised.at("p").get_int64()),
  m_ram(serialised.at("ram")) {}

state_t::state_t(const boost::json::value& serialised)
: state_t(serialised.as_object()) {}

#endif

#ifdef USE_NLOHMANN_JSON

state_t::state_t(const nlohmann::json& serialised)
: m_pc(serialised["pc"].get<uint16_t>()),
  m_s(serialised["s"].get<uint8_t>()),
  m_a(serialised["a"].get<uint8_t>()),
  m_x(serialised["x"].get<uint8_t>()),
  m_y(serialised["y"].get<uint8_t>()),
  m_p(serialised["p"].get<uint8_t>()),
  m_ram(serialised["ram"]) {}

#endif

#ifdef USE_JSONCPP_JSON

state_t::state_t(const Json::Value& serialised)
: m_pc(serialised["pc"].asUInt()),
  m_s(serialised["s"].asUInt()),
  m_a(serialised["a"].asUInt()),
  m_x(serialised["x"].asUInt()),
  m_y(serialised["y"].asUInt()),
  m_p(serialised["p"].asUInt()),
  m_ram(serialised["ram"]) {}

#endif
