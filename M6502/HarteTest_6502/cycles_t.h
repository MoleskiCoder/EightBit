#pragma once

#include "simdjson/simdjson.h"

#include "array_t.h"

class cycles_t final : public array_t {
public:
	cycles_t(simdjson::dom::array input) noexcept;
};
