#include "stdafx.h"
#include "state_t.h"
#include <cassert>

void state_t::initialise(const boost::json::object& serialised) {

    assert(!initialised());

    m_pc = get_uint16(serialised, "pc");
    m_s = get_uint8(serialised, "s");
    m_a = get_uint8(serialised, "a");
    m_x = get_uint8(serialised, "x");
    m_y = get_uint8(serialised, "y");
    m_p = get_uint8(serialised, "p");

    const auto& ram_entries = get_array(serialised, "ram");
    for (const auto& ram_entry : ram_entries) {
        assert(ram_entry.is_array());
        const auto& ram_entry_array = ram_entry.as_array();
        assert(ram_entry_array.size() == 2);
        const auto address = get_uint16(ram_entry_array[0]);
        const auto value = get_uint8(ram_entry_array[1]);
        m_ram[address] = value;
    }

    m_initialised = true;
}

state_t::state_t() {}

state_t::state_t(const boost::json::object& serialised) {
    initialise(serialised);
    assert(initialised());
}

state_t::state_t(const boost::json::value& serialised) {
    assert(serialised.is_object());
    initialise(serialised.get_object());
    assert(initialised());
}
