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

#ifdef USE_BOOST_JSON

test_t::test_t(const boost::json::object& serialised)
: m_name(serialised.at("name").as_string()),
  m_initial_state(serialised.at("initial")),
  m_final_state(serialised.at("final")),
  m_cycles(serialised.at("cycles")) {}

test_t::test_t(const boost::json::value& serialised)
: test_t(serialised.as_object()) {}

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



    //m_name = serialised["name"].asString();
    //m_initial_state = state_t(serialised["initial"]);
    //m_final_state = state_t(serialised["final"]);

    //const auto& cycles_array = serialised["cycles"];
    //m_cycles.reserve(cycles_array.size());

    //for (const auto& cycles_entry : cycles_array) {
    //    assert(cycles_entry.size() == 3);
    //    const auto address = cycles_entry[0].asUInt();
    //    const auto contents = cycles_entry[1].asUInt();
    //    const auto action = to_action(cycles_entry[2].asString());
    //    m_cycles.push_back({ address, contents, action });
    //}
//}

#endif
