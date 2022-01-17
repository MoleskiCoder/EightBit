#pragma once

#include <Bus.h>
#include <Ram.h>
#include <mos6502.h>

class TestRunner final : public EightBit::Bus {
private:
    EightBit::Ram m_ram = 0x10000;
    EightBit::MOS6502 m_cpu = { *this };

protected:
    EightBit::MemoryMapping mapping(uint16_t address) noexcept final;

public:
    TestRunner();

    void raisePOWER() noexcept final;
    void lowerPOWER() noexcept final;

    void initialise() final;

    [[nodiscard]] constexpr auto& RAM() noexcept { return m_ram; }
    [[nodiscard]] constexpr auto& CPU() noexcept { return m_cpu; }
};
