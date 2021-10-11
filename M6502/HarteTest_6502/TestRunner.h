#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <Bus.h>
#include <Ram.h>
#include <mos6502.h>

#include "test_t.h"

class TestRunner final : public EightBit::Bus {
private:
    EightBit::Ram m_ram = 0x10000;
    EightBit::MOS6502 m_cpu = { *this };
    const test_t& m_test;

    std::ostringstream m_os;
    std::vector<std::string> m_messages;

    test_t::events_t m_actualEvents;
    bool m_event_count_mismatch = false;

    void initialiseState();
    [[nodiscard]] bool checkState();

    void raise(std::string what, uint16_t expected, uint16_t actual);
    void raise(std::string what, uint8_t expected, uint8_t actual);
    void raise(std::string what, test_t::action expected, test_t::action actual);

    template<class T>
    bool check(std::string what, T expected, T actual) {
        const auto success = actual == expected;
        if (!success)
            raise(what, expected, actual);
        return success;
    }

    bool check(std::string what, uint16_t address, uint8_t expected, uint8_t actual);


    void addActualEvent(test_t::action action, uint16_t address, uint8_t value);

    [[nodiscard]] auto& os() { return m_os; }

protected:
    virtual EightBit::MemoryMapping mapping(uint16_t address) noexcept final;

public:
    TestRunner(const test_t& test);

    virtual void raisePOWER() final;
    virtual void lowerPOWER() final;

    virtual void initialise() final;

    [[nodiscard]] constexpr auto& RAM() noexcept { return m_ram; }
    [[nodiscard]] constexpr auto& CPU() noexcept { return m_cpu; }
    [[nodiscard]] constexpr const auto& test() const noexcept { return m_test; }
    [[nodiscard]] constexpr const auto& messages() const noexcept { return m_messages; }

    [[nodiscard]] bool check();
};
