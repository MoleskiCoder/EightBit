#pragma once

#include <cstdint>
#include <unordered_map>
#include <boost/json.hpp>

#include "json_t.h"

class state_t final : public json_t {
private:
    bool m_initialised = false;

    uint16_t m_pc = 0xffff;
    uint8_t m_s = 0xff;
    uint8_t m_a = 0xff;
    uint8_t m_x = 0xff;
    uint8_t m_y = 0xff;
    uint8_t m_p = 0xff;
    std::unordered_map<uint16_t, uint8_t> m_ram;

    constexpr auto initialised() const noexcept { return m_initialised; }

    void initialise(const boost::json::object& serialised);

public:
    state_t();
    state_t(const boost::json::object& serialised);
    state_t(const boost::json::value& serialised);

    constexpr auto pc() const noexcept { return m_pc; }
    constexpr auto s() const noexcept { return m_s; }
    constexpr auto a() const noexcept { return m_a; }
    constexpr auto x() const noexcept { return m_x; }
    constexpr auto y() const noexcept { return m_y; }
    constexpr auto p() const noexcept { return m_p; }
    constexpr const auto& ram() const noexcept { return m_ram; }
};
