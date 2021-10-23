#include "stdafx.h"
#include "array_t.h"

array_t::array_t(const simdjson::dom::array input) noexcept
: m_raw(input) {}
