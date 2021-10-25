#include "stdafx.h"
#include "test_t.h"

test_t::test_t() noexcept {}

test_t::test_t(const simdjson::dom::element input) noexcept
: element_t(input) {}
