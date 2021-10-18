#include "stdafx.h"
#include "TestRunner.h"

#include <Disassembly.h>

TestRunner::TestRunner(const test_t& test)
: m_test(test) {}

EightBit::MemoryMapping TestRunner::mapping(uint16_t address) noexcept {
    return { RAM(), 0x0000, 0xffff, EightBit::MemoryMapping::AccessLevel::ReadWrite };
}

void TestRunner::raisePOWER() {
    EightBit::Bus::raisePOWER();
    CPU().raisePOWER();
    CPU().raiseRESET();
    CPU().raiseINT();
    CPU().raiseNMI();
    CPU().raiseSO();
    CPU().raiseRDY();
}

void TestRunner::lowerPOWER() {
    CPU().lowerPOWER();
    EightBit::Bus::lowerPOWER();
}

void TestRunner::addActualCycle(const cycle_t& value) {
    m_actualCycles.add(value);
}

void TestRunner::addActualCycle(uint16_t address, uint8_t value, cycle_t::action_t action) {
    addActualCycle({ address, value, action });
}

void TestRunner::addActualCycle(EightBit::register16_t address, uint8_t value, cycle_t::action_t action) {
    addActualCycle(address.word, value, action);
}

void TestRunner::addActualReadCycle(EightBit::register16_t address, uint8_t value) {
    addActualCycle(address, value, cycle_t::action_t::read);
}

void TestRunner::addActualWriteCycle(EightBit::register16_t address, uint8_t value) {
    addActualCycle(address, value, cycle_t::action_t::write);
}

void TestRunner::dumpCycles(std::string which, const cycles_t& events) {
    m_messages.push_back(which);
    dumpCycles(events);
}

void TestRunner::dumpCycles(const cycles_t& cycles) {
    os() << std::hex << std::uppercase << std::setfill('0');
    for (const auto& cycle: cycles)
        dumpCycle(cycle);
}

void TestRunner::dumpCycle(const cycle_t& cycle) {
    os()
        << "Address: " << std::setw(4) << cycle.address()
        << ", value: " << std::setw(2) << (int)cycle.value()
        << ", action: " << cycle_t::to_string(cycle.action());
    m_messages.push_back(os().str());
    os().str("");
}

void TestRunner::initialise() {

    ReadByte.connect([this](EightBit::EventArgs&) {
        addActualReadCycle(ADDRESS(), DATA());
    });

    WrittenByte.connect([this](EightBit::EventArgs&) {
        addActualWriteCycle(ADDRESS(), DATA());
    });

    os() << std::hex << std::uppercase << std::setfill('0');
}

void TestRunner::raise(std::string what, uint16_t expected, uint16_t actual) {
    os()
        << std::setw(4)
        << what
        << ": expected: " << (int)expected
        << ", actual: " << (int)actual;
    m_messages.push_back(os().str());
    os().str("");
}

void TestRunner::raise(std::string what, uint8_t expected, uint8_t actual) {
    os()
        << std::setw(2)
        << what
        << ": expected: " << (int)expected
            << "(" << EightBit::Disassembly::dump_Flags(expected) << ")"
        << ", actual: " << (int)actual
            << "(" << EightBit::Disassembly::dump_Flags(actual) << ")";
    m_messages.push_back(os().str());
    os().str("");
}

void TestRunner::raise(std::string what, cycle_t::action_t expected, cycle_t::action_t actual) {
    os()
        << what
        << ": expected: " << cycle_t::to_string(expected)
        << ", actual: " << cycle_t::to_string(actual);
    m_messages.push_back(os().str());
    os().str("");
}

bool TestRunner::check(std::string what, uint16_t address, uint8_t expected, uint8_t actual) {
    const auto success = actual == expected;
    if (!success) {
        os() << what << ": " << std::setw(4) << (int)address;
        raise(os().str(), expected, actual);
        os().str("");
    }
    return success;
}

void TestRunner::initialiseState() {

    const auto& starting = test().initial_state();

    CPU().PC().word = starting.pc();
    CPU().S() = starting.s();
    CPU().A() = starting.a();
    CPU().X() = starting.x();
    CPU().Y() = starting.y();
    CPU().P() = starting.p();
    const auto& ram = starting.ram();
    for (const auto& entry : ram) {
        const auto [address, value] = entry;
        RAM().poke(address, value);
    }

    m_actualCycles.clear();
}

bool TestRunner::checkState() {

    const auto& finished = test().final_state();

    const auto& expected_cycles = test().cycles();
    const auto& actual_cycles = m_actualCycles;
    m_cycle_count_mismatch = expected_cycles.size() != actual_cycles.size();
    if (m_cycle_count_mismatch)
        return false;

    for (int i = 0; i < expected_cycles.size(); ++i) {
        const auto& expected = expected_cycles[i];
        const auto& actual = actual_cycles[i];   // actual could be less than expected
        check("Cycle address", expected.address(), actual.address());
        check("Cycle value", expected.value(), actual.value());
        check("Cycle action", expected.action(), actual.action());
    }

    const auto pc_good = check("PC", finished.pc(), CPU().PC().word);
    const auto s_good = check("S", finished.s(), CPU().S());
    const auto a_good = check("A", finished.a(), CPU().A());
    const auto x_good = check("X", finished.x(), CPU().X());
    const auto y_good = check("Y", finished.y(), CPU().Y());
    const auto p_good = check("P", finished.p(), CPU().P());

    const auto& ram = finished.ram();
    bool ram_problem = false;
    for (const auto& entry : ram) {
        const auto [address, value] = entry;
        const auto ram_good = check("RAM",  address, value, RAM().peek(address));
        if (!ram_good && !ram_problem)
            ram_problem = true;
    }

    return pc_good && s_good && a_good && x_good && y_good && p_good && !ram_problem;
}

bool TestRunner::check() {
    initialise();
    raisePOWER();
    initialiseState();
    const int cycles = CPU().step();
    const auto valid = checkState();
    if (m_cycle_count_mismatch) {
        if (cycles == 1) {
            m_messages.push_back("Unimplemented");
        } else {

            os()
                << std::dec << std::setfill(' ')
                << "Stepped cycles: " << cycles
                << ", expected events: " << test().cycles().size()
                << ", actual events: " << m_actualCycles.size();
            m_messages.push_back(os().str());
            os().str("");

            dumpCycles("-- Expected cycles", test().cycles());
            dumpCycles("-- Actual cycles", m_actualCycles);
        }
    }
    lowerPOWER();
    return valid;
}
