#pragma once

#include <cstdint>

#include "simdjson/simdjson.h"

#include "ram_t.h"

class state_t final {
private:
    const simdjson::dom::element m_raw;

    [[nodiscard]] auto at(std::string key) const noexcept { return m_raw[key]; }
    [[nodiscard]] auto integer_at(std::string key) const noexcept { return (int64_t)at(key); }
    [[nodiscard]] auto address_at(std::string key) const noexcept { return (uint16_t)integer_at(key); }
    [[nodiscard]] auto byte_at(std::string key) const noexcept { return (uint8_t)integer_at(key); }
    [[nodiscard]] auto array_at(std::string key) const noexcept { return at(key).get_array(); }

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
