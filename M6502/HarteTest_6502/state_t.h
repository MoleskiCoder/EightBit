#pragma once

#include <cstdint>

#include "simdjson/simdjson.h"

#include "ram_t.h"

class state_t final {
private:
    const simdjson::dom::element m_raw;

public:
    state_t(simdjson::dom::element input);

    [[nodiscard]] auto pc() const noexcept { return (uint16_t)(int64_t)m_raw["pc"]; }
    [[nodiscard]] auto s() const noexcept { return (uint8_t)(int64_t)m_raw["s"]; }
    [[nodiscard]] auto a() const noexcept { return (uint8_t)(int64_t)m_raw["a"]; }
    [[nodiscard]] auto x() const noexcept { return (uint8_t)(int64_t)m_raw["x"]; }
    [[nodiscard]] auto y() const noexcept { return (uint8_t)(int64_t)m_raw["y"]; }
    [[nodiscard]] auto p() const noexcept { return (uint8_t)(int64_t)m_raw["p"]; }
    [[nodiscard]] const auto ram() const noexcept { return ram_t(m_raw["ram"].get_array()); }
};
