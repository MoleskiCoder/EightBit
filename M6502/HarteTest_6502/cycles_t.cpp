#include "stdafx.h"
#include "cycles_t.h"

cycles_t::cycles_t(size_t reserved) {
    m_cycles.reserve(reserved);
}

void cycles_t::add(const cycle_t& cycle) {
    assert(m_cycles.capacity() >= (m_cycles.size() + 1));
    m_cycles.push_back(cycle);
}

cycles_t::cycles_t(simdjson::dom::array input) {
    assert(m_cycles.empty());
    m_cycles.reserve(input.size());
    for (const auto& entry : input)
        add(entry);
}
