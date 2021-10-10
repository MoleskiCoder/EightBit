#include "stdafx.h"
#include "test_t.h"
#include <cassert>
#include <stdexcept>

test_t::action test_t::to_action(std::string value) {
    if (value == "read")
        return action::read;
    if (value == "write")
        return action::write;
    throw new std::out_of_range("Unknown action");
}

std::string test_t::to_string(action value) {
    if (value == action::read)
        return "read";
    if (value == action::write)
        return "write";
    throw new std::out_of_range("Unknown action");
}

void test_t::initialise(const boost::json::object& serialised) {

    m_name = get_string(serialised, "name");
    m_initial_state = state_t(get_value(serialised, "initial"));
    m_final_state = state_t(get_value(serialised, "final"));

    const auto& cycles_array = get_array(serialised, "cycles");
    m_cycles.reserve(cycles_array.size());

    for (const auto& cycles_entry : cycles_array) {
        const auto& cycle_array = get_array(cycles_entry);
        assert(cycle_array.size() == 3);
        const auto address = get_uint16(cycle_array[0]);
        const auto contents = get_uint8(cycle_array[1]);
        const auto action = to_action((std::string)get_string(cycle_array[2]));
        m_cycles.push_back( { address, contents, action } );
    }
}

test_t::test_t(const boost::json::object& serialised) {
    initialise(serialised);
}

test_t::test_t(const boost::json::value& serialised) {
    assert(serialised.is_object());
    initialise(serialised.get_object());
}
