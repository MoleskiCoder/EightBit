#include "stdafx.h"
#include "TestRunner.h"

#include <sstream>
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

void TestRunner::addActualEvent(test_t::action action, uint16_t address, uint8_t value) {
    m_actualEvents.push_back( { address, value, action } );
}

void TestRunner::initialise() {

    ReadByte.connect([this](EightBit::EventArgs&) {
        addActualEvent(test_t::action::read, ADDRESS().word, DATA());
    });

    WrittenByte.connect([this](EightBit::EventArgs&) {
        addActualEvent(test_t::action::write, ADDRESS().word, DATA());
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

void TestRunner::raise(std::string what, test_t::action expected, test_t::action actual) {
    os()
        << what
        << ": expected: " << test_t::to_string(expected)
        << ", actual: " << test_t::to_string(actual);
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

    m_actualEvents.clear();
}

bool TestRunner::checkState() {

    const auto& finished = test().final_state();

    const auto& expected_events = test().cycles();
    const auto& actual_events = m_actualEvents;
    m_event_count_mismatch = expected_events.size() != actual_events.size();
    if (m_event_count_mismatch)
        return false;

    for (int i = 0; i < expected_events.size(); ++i) {
        const auto& expected = expected_events[i];
        const auto [expectedAddress, expectedContents, expectedAction] = expected;
        const auto& actual = actual_events.at(i);   // actual could be less than expected
        const auto [actualAddress, actualContents, actualAction] = actual;
        check("Event action", expectedAction, actualAction);
        check("Event address", expectedAddress, actualAddress);
        check("Event contents", expectedContents, actualContents);
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
    if (m_event_count_mismatch) {
        std::ostringstream os;
        os
            << "Stepped cycles: " << cycles
            << ", expected events: " << test().cycles().size()
            << ", actual events: " << m_actualEvents.size();
        m_messages.push_back(os.str());
    }
    lowerPOWER();
    return valid;
}
