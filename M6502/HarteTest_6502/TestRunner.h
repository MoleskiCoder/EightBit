#pragma once

#include <cstdint>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <Bus.h>
#include <Ram.h>
#include <mos6502.h>

class TestRunner final : public EightBit::Bus {
private:
    static std::set<uint8_t> m_undocumented_opcodes;
    static bool m_undocumented_opcodes_initialised;

    EightBit::Ram m_ram = 0x10000;
    EightBit::MOS6502 m_cpu = { *this };

protected:
    virtual EightBit::MemoryMapping mapping(uint16_t address) noexcept final;

public:
    TestRunner();

    virtual void raisePOWER() final;
    virtual void lowerPOWER() final;

    virtual void initialise() final;

    [[nodiscard]] constexpr auto& RAM() noexcept { return m_ram; }
    [[nodiscard]] constexpr auto& CPU() noexcept { return m_cpu; }
};
