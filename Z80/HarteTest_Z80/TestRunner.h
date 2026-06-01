#pragma once

#include <Bus.h>
#include <Ram.h>
#include <Z80.h>

class TestRunner final : public EightBit::Bus {
private:
    EightBit::Ram m_ram = 0x10000;
	EightBit::InputOutput m_ports;
    EightBit::Z80 m_cpu = { *this, m_ports };
	EightBit::MemoryMapping m_mapping = { m_ram, 0x0000, 0xffff, EightBit::MemoryMapping::AccessLevel::ReadWrite };

protected:
	const EightBit::MemoryMapping& mapping(uint16_t address) noexcept final { return m_mapping; }

public:
    TestRunner();

    void raisePOWER() noexcept final;
    void lowerPOWER() noexcept final;

    void initialise() noexcept final;

    [[nodiscard]] constexpr auto& RAM() noexcept { return m_ram; }
    [[nodiscard]] constexpr auto& ports() noexcept { return m_ports; }
    [[nodiscard]] constexpr auto& CPU() noexcept { return m_cpu; }
};
