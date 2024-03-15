#pragma once

#include <cstdint>

#include <Disassembly.h>
#include <Symbols.h>

#include "cycles_t.h"
#include "cycle_t.h"
#include "test_t.h"

#include "TestRunner.h"

class checker_t {
private:
    TestRunner& m_runner;
    EightBit::Symbols m_symbols;
    EightBit::Disassembly m_disassembler = { m_runner, m_runner.CPU(), m_symbols };

    std::ostringstream m_os;
    std::vector<std::string> m_messages;

    const std::string m_read_action = { "read" };
    const std::string m_write_action = { "write" };

    typedef std::tuple<uint16_t, uint8_t, std::string_view> actual_cycle_t;
    typedef std::vector<actual_cycle_t> actual_cycles_t;
    actual_cycles_t m_actualCycles;
    bool m_cycle_count_mismatch = false;

    int m_cycles = 0;
    bool m_valid = true;

    [[nodiscard]] constexpr auto& os() noexcept { return m_os; }

    [[nodiscard]] constexpr auto& runner() noexcept { return m_runner; }

    [[nodiscard]] bool checkState(test_t test);

    void pushCurrentMessage();

    void raise(std::string_view what, uint16_t expected, uint16_t actual);
    void raise(std::string_view what, uint8_t expected, uint8_t actual);
    void raise(std::string_view what, std::string_view expected, std::string_view actual);

    template<class T>
    constexpr bool check(std::string_view what, T expected, T actual) noexcept {
        const auto success = actual == expected;
        if (!success)
            raise(what, expected, actual);
        return success;
    }

    bool check(std::string_view what, uint16_t address, uint8_t expected, uint8_t actual);

    void addActualCycle(uint16_t address, uint8_t value, std::string_view action);
    void addActualCycle(EightBit::register16_t address, uint8_t value, std::string_view action);

    void addActualReadCycle(EightBit::register16_t address, uint8_t value);
    void addActualWriteCycle(EightBit::register16_t address, uint8_t value);

    void add_disassembly(uint16_t address);

    template<class T>
    void dumpCycle(const uint16_t address, const uint8_t value, const T action) {
        m_os
            << std::setfill('0') << std::hex
            << "Address: " << std::setw(4) << (int)address
            << ", value: " << std::setw(2) << (int)value
            << ", action: " << action;
        pushCurrentMessage();
    }

    void dumpCycles(std::string which, cycles_t cycles);
    void dumpCycles(cycles_t cycles);
    void dumpCycle(cycle_t cycle);

    void dumpCycles(std::string which, const actual_cycles_t& cycles);
    void dumpCycles(const actual_cycles_t& cycles);
    void dumpCycle(const actual_cycle_t& cycle);

    void initialiseState(test_t test);

public:
    checker_t(TestRunner& runner);

    [[nodiscard]] constexpr auto cycles() const noexcept { return m_cycles; }
    [[nodiscard]] constexpr auto valid() const noexcept { return m_valid; }
    [[nodiscard]] constexpr auto invalid() const noexcept { return !valid(); }
    [[nodiscard]] constexpr auto unimplemented() const noexcept { return invalid() && m_cycle_count_mismatch && (cycles() == 1); }
    [[nodiscard]] constexpr auto implemented() const noexcept { return !unimplemented(); }

    [[nodiscard]] constexpr const auto& messages() const noexcept { return m_messages; }

    void initialise();

    [[nodiscard]] std::string disassemble(uint16_t address);

    void check(test_t test);
};
