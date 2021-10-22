#include "stdafx.h"
#include "element_t.h"

element_t::element_t(const simdjson::dom::element input) noexcept
: m_raw(input) {}
