#pragma once

#include <Bus.h>
#include <Ram.h>
#include <mos6502.h>

#include "test_t.h"

class TestRunner final : public EightBit::Bus {
private:
    EightBit::Ram m_ram = 0x10000;
    EightBit::MOS6502 m_cpu = { *this };
    const test_t& m_test;

    test_t::events_t m_actualEvents;

    void initialiseState();
    void verifyState();

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

    void addActualEvent(test_t::action action, uint16_t address, uint8_t value);

protected:
    virtual EightBit::MemoryMapping mapping(uint16_t address) noexcept final;

public:
    TestRunner(const test_t& test);

    virtual void raisePOWER() final;
    virtual void lowerPOWER() final;

    virtual void initialise() final;

    constexpr auto& RAM() noexcept { return m_ram; }
    constexpr auto& CPU() noexcept { return m_cpu; }
    constexpr const auto& test() const noexcept { return m_test; }

    void run();
};
