#include "stdafx.h"
#include "test_t.h"

test_t::test_t(const simdjson::dom::element input) noexcept
: m_raw(input) {}
