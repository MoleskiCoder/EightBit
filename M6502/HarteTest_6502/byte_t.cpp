#include "stdafx.h"
#include "byte_t.h"

byte_t::byte_t(simdjson::dom::array input) noexcept
: array_t(input) {}
