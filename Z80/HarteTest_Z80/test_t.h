#pragma once

#include <optional>

#include "simdjson/simdjson.h"

#include "element_t.h"
#include "cycles_t.h"
#include "ports_t.h"
#include "state_t.h"

class test_t final : public element_t {
public:
    test_t() noexcept {}

    test_t(const simdjson::dom::element input) noexcept
    : element_t(input) {}

    [[nodiscard]] auto name() const noexcept { return at("name"); }
    [[nodiscard]] auto initial() const noexcept { return state_t(at("initial")); }
    [[nodiscard]] auto final() const noexcept { return state_t(at("final")); }
    [[nodiscard]] auto cycles() const noexcept { return cycles_t(array_at("cycles")); }

    [[nodiscard]] auto ports() const noexcept {
		std::optional<ports_t> returned;
        auto possible = array_at("ports");
		if (possible.has_value())
			returned.emplace(ports_t(possible.value()));
        return returned;;
    }
};
