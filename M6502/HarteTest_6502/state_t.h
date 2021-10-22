#pragma once

#include <string>

#include "simdjson/simdjson.h"

#include "element_t.h"
#include "ram_t.h"

class state_t final : public element_t {
private:
    [[nodiscard]] auto address_at(std::string key) const noexcept { return (uint16_t)integer_at(key); }
    [[nodiscard]] auto byte_at(std::string key) const noexcept { return (uint8_t)integer_at(key); }

public:
    state_t(simdjson::dom::element input) noexcept;

    [[nodiscard]] auto pc() const noexcept { return address_at("pc"); }
    [[nodiscard]] auto s() const noexcept { return byte_at("s"); }
    [[nodiscard]] auto a() const noexcept { return byte_at("a"); }
    [[nodiscard]] auto x() const noexcept { return byte_at("x"); }
    [[nodiscard]] auto y() const noexcept { return byte_at("y"); }
    [[nodiscard]] auto p() const noexcept { return byte_at("p"); }
    [[nodiscard]] auto ram() const noexcept { return ram_t(array_at("ram")); }
};
