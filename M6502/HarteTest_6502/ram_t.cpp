#include "stdafx.h"
#include "ram_t.h"

ram_t::ram_t(simdjson::dom::array input) noexcept
: m_raw(input) {}
