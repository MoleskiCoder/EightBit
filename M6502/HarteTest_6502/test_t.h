#pragma once

#include "simdjson/simdjson.h"

#include "element_t.h"
#include "cycles_t.h"
#include "state_t.h"

class test_t final : public element_t {
public:
    test_t() noexcept;
    test_t(simdjson::dom::element input) noexcept;

    [[nodiscard]] auto name() const noexcept { return at("name"); }
    [[nodiscard]] auto initial() const noexcept { return state_t(at("initial")); }
    [[nodiscard]] auto final() const noexcept { return state_t(at("final")); }
    [[nodiscard]] auto cycles() const noexcept { return cycles_t(array_at("cycles")); }
};
