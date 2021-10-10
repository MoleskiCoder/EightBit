#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <tuple>
#include <boost/json.hpp>

#include "state_t.h"
#include "json_t.h"

class test_t final : public json_t {
public:
    enum class action { read, write };
    
    typedef std::tuple<uint16_t, uint8_t, action> event_t;  // address, contents, action
    typedef std::vector<event_t> events_t;

    static action to_action(std::string value);
    static std::string to_string(action value);

private:
    std::string m_name;
    state_t m_initial_state;
    state_t m_final_state;
    events_t m_cycles; 

    void initialise(const boost::json::object& serialised);

public:
    test_t(const boost::json::object& serialised);
    test_t(const boost::json::value& serialised);

    constexpr const auto& name() const noexcept { return m_name; }
    constexpr const auto& initial_state() const noexcept { return m_initial_state; }
    constexpr const auto& final_state() const noexcept { return m_final_state; }
    constexpr const auto& cycles() const noexcept { return m_cycles; }
};