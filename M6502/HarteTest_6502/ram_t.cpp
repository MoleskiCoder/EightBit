#include "stdafx.h"
#include "ram_t.h"

void ram_t::add(byte_t byte) {
    assert(m_bytes.capacity() >= (m_bytes.size() + 1));
    m_bytes.push_back(byte);
}

ram_t::ram_t(simdjson::dom::array input) {
    assert(m_bytes.empty());
    m_bytes.reserve(input.size());
    for (auto entry : input)
        add(entry);
}
