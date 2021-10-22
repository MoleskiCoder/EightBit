#include "stdafx.h"
#include "cycle_t.h"

cycle_t::cycle_t(simdjson::dom::array input) noexcept
: byte_t(input) {}
