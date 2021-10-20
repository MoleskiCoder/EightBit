#include "stdafx.h"
#include "byte_t.h"

byte_t::byte_t(simdjson::dom::element input) noexcept
: byte_t(input.get_array()) {}

byte_t::byte_t(simdjson::dom::array input) noexcept
: m_raw(input) {}
