#pragma once

#include <cstdint>

#include "simdjson/simdjson.h"

#include "ram_t.h"

class state_t final {
private:
    uint16_t m_pc = 0xffff;
    uint8_t m_s = 0xff;
    uint8_t m_a = 0xff;
    uint8_t m_x = 0xff;
    uint8_t m_y = 0xff;
    uint8_t m_p = 0xff;
    ram_t m_ram;

public:
    state_t(simdjson::dom::element serialised);

    [[nodiscard]] constexpr auto pc() const noexcept { return m_pc; }
    [[nodiscard]] constexpr auto s() const noexcept { return m_s; }
    [[nodiscard]] constexpr auto a() const noexcept { return m_a; }
    [[nodiscard]] constexpr auto x() const noexcept { return m_x; }
    [[nodiscard]] constexpr auto y() const noexcept { return m_y; }
    [[nodiscard]] constexpr auto p() const noexcept { return m_p; }
    [[nodiscard]] constexpr const auto& ram() const noexcept { return m_ram; }
};
