#include "stdafx.h"
#include "TestRunner.h"

#include <iostream>
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

}

void TestRunner::raise(std::string what, uint16_t expected, uint16_t actual) {
    std::cout
        << "** Failure: " << what
        << ": expected: " << EightBit::Disassembly::dump_WordValue(expected)
        << ", actual: " << EightBit::Disassembly::dump_WordValue(actual)
        << "\n";
}

void TestRunner::raise(std::string what, uint8_t expected, uint8_t actual) {
    std::cout
        << "** Failure: " << what
        << ": expected: " << EightBit::Disassembly::dump_ByteValue(expected)
            << "(" << EightBit::Disassembly::dump_Flags(expected) << ")"
        << ", actual: " << EightBit::Disassembly::dump_ByteValue(actual)
            << "(" << EightBit::Disassembly::dump_Flags(actual) << ")"
        << "\n";
}

void TestRunner::raise(std::string what, test_t::action expected, test_t::action actual) {
    std::cout
        << "** Failure: " << what
        << ": expected: " << test_t::to_string(expected)
        << ", actual: " << test_t::to_string(actual)
        << "\n";
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

void TestRunner::verifyState() {

    const auto& finished = test().final_state();

    const auto& expected_events = test().cycles();
    const auto& actual_events = m_actualEvents;
    if (expected_events.size() != actual_events.size()) {
        //std::cout << "** event count mismatch" << "\n";
        return;
    }

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
        const auto ram_good = check("RAM: " + EightBit::Disassembly::dump_WordValue(address), value, RAM().peek(address));
        if (!ram_good && !ram_problem)
            ram_problem = true;
    }

    const auto good = pc_good && s_good && a_good && x_good && y_good && p_good && !ram_problem;
    std::cout << (good ? "+" : "-");
}

void TestRunner::run() {
    initialise();
    raisePOWER();
    initialiseState();
    const int cycles = CPU().step();
    verifyState();
    lowerPOWER();
}
