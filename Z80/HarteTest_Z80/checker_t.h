#pragma once

#include <cstdint>
#include <optional>

#include <Disassembler.h>

#include "cycles_t.h"
#include "cycle_t.h"
#include "test_t.h"

#include "TestRunner.h"

class checker_t {
private:
    TestRunner& m_runner;
    EightBit::Disassembler m_disassembler = { m_runner };

    std::ostringstream m_os;
    std::vector<std::string> m_messages;

    typedef std::tuple<uint16_t, uint8_t, std::string> actual_cycle_t;
    typedef std::vector<actual_cycle_t> actual_cycles_t;
    actual_cycles_t m_actualCycles;
    bool m_cycle_count_mismatch = false;

    int m_cycles = 0;
    bool m_valid = true;

    [[nodiscard]] constexpr auto& os() noexcept { return m_os; }

    [[nodiscard]] constexpr auto& runner() noexcept { return m_runner; }

    [[nodiscard]] bool checkState(test_t test);

    void pushCurrentMessage();

    void raise(const std::string& what, uint16_t expected, uint16_t actual);
    void raise(const std::string& what, uint8_t expected, uint8_t actual);
    void raiseFlags(const std::string& what, uint8_t expected, uint8_t actual);
    void raise(const std::string& what, const std::string& expected, const std::string& actual);

    template<class T>
    constexpr bool check(const std::string& what, T expected, T actual) noexcept {
        const auto success = actual == expected;
        if (!success)
            raise(what, expected, actual);
        return success;
    }

    bool check(const std::string& what, uint16_t address, uint8_t expected, uint8_t actual);

    void addActualCycle(uint16_t address, uint8_t value, const std::string& action);
    void addActualCycle(EightBit::register16_t address, uint8_t value, const std::string& action);

    void add_disassembly(uint16_t address);

    template<class T>
    void dumpCycle(const uint16_t address, const std::optional<uint8_t> value, const T action) {
        if (value.has_value())
            m_os
                << std::setfill('0') << std::hex
                << "Address: " << std::setw(4) << (int)address
			    << ", value: " << std::setw(2) << (int)value.value()
                << ", action: " << action;
        else
            m_os
                << std::setfill('0') << std::hex
                << "Address: " << std::setw(4) << (int)address
                << "           "
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
    void initialiseState(state_t state, std::optional<ports_t> ports);

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
