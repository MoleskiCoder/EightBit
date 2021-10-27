#pragma once

#include <cstdint>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <Bus.h>
#include <Ram.h>
#include <mos6502.h>
#include <Disassembly.h>
#include <Symbols.h>

#include "test_t.h"
#include "cycle_t.h"

class TestRunner final : public EightBit::Bus {
private:
    static std::set<uint8_t> m_undocumented_opcodes;
    static bool m_undocumented_opcodes_initialised;

    EightBit::Ram m_ram = 0x10000;
    EightBit::MOS6502 m_cpu = { *this };
    EightBit::Symbols m_symbols;
    EightBit::Disassembly m_disassembler = { *this, m_cpu, m_symbols };

    test_t m_test;

    std::ostringstream m_os;
    std::vector<std::string> m_messages;

    typedef std::tuple<uint16_t, uint8_t, std::string> actual_cycle_t;
    typedef std::vector<actual_cycle_t> actual_cycles_t;
    actual_cycles_t m_actualCycles;
    bool m_cycle_count_mismatch = false;

    int m_cycles = 0;
    bool m_valid = true;
    bool m_undocumented = false;

    void seedUndocumentedOpcodes();
    void initialiseState();
    [[nodiscard]] bool checkState();

    void pushCurrentMessage();

    void raise(std::string_view what, uint16_t expected, uint16_t actual);
    void raise(std::string_view what, uint8_t expected, uint8_t actual);
    void raise(std::string_view what, std::string_view expected, std::string_view actual);

    template<class T>
    bool check(std::string_view what, T expected, T actual) {
        const auto success = actual == expected;
        if (!success)
            raise(what, expected, actual);
        return success;
    }

    bool check(std::string_view what, std::string_view expected, std::string_view actual) {
        const auto success = actual == expected;
        if (!success)
            raise(what, expected, actual);
        return success;
    }

    bool check(std::string_view what, uint16_t address, uint8_t expected, uint8_t actual);

    void addActualCycle(const actual_cycle_t& value);
    void addActualCycle(uint16_t address, uint8_t value, std::string_view action);
    void addActualCycle(EightBit::register16_t address, uint8_t value, std::string_view action);

    void addActualReadCycle(EightBit::register16_t address, uint8_t value);
    void addActualWriteCycle(EightBit::register16_t address, uint8_t value);

    void disassemble(uint16_t address);

    template<class T>
    void dumpCycle(const uint16_t address, const uint8_t value, const T action) {
        m_os
            << std::setfill('0') << std::hex
            << "Address: " << std::setw(4) << (int)address
            << ", value: " << std::setw(2) << (int)value
            << ", action: " << action;
        pushCurrentMessage();
    }

    void dumpCycles(std::string_view which, cycles_t cycles);
    void dumpCycles(cycles_t cycles);
    void dumpCycle(cycle_t cycle);

    void dumpCycles(std::string_view which, const actual_cycles_t& cycles);
    void dumpCycles(const actual_cycles_t& cycles);
    void dumpCycle(const actual_cycle_t& cycle);

    [[nodiscard]] auto& os() { return m_os; }

protected:
    virtual EightBit::MemoryMapping mapping(uint16_t address) noexcept final;

public:
    TestRunner();

    virtual void raisePOWER() final;
    virtual void lowerPOWER() final;

    virtual void initialise() final;

    [[nodiscard]] constexpr auto& RAM() noexcept { return m_ram; }
    [[nodiscard]] constexpr auto& CPU() noexcept { return m_cpu; }
    [[nodiscard]] auto test() const noexcept { return m_test; }
    [[nodiscard]] constexpr const auto& messages() const noexcept { return m_messages; }

    [[nodiscard]] constexpr auto cycles() const noexcept { return m_cycles; }
    [[nodiscard]] constexpr auto valid() const noexcept { return m_valid; }
    [[nodiscard]] constexpr auto invalid() const noexcept { return !valid(); }
    [[nodiscard]] constexpr auto unimplemented() const noexcept { return invalid() && m_cycle_count_mismatch && (cycles() == 1); }
    [[nodiscard]] constexpr auto implemented() const noexcept { return !unimplemented(); }
    [[nodiscard]] constexpr auto undocumented() const noexcept { return m_undocumented; }
    [[nodiscard]] constexpr auto documented() const noexcept { return !undocumented(); }

    void check(test_t updated);
};
