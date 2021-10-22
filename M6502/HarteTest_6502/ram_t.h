#pragma once

#include "simdjson/simdjson.h"

#include "array_t.h"

class ram_t final : public array_t {
public:
    ram_t(simdjson::dom::array input) noexcept;
};
