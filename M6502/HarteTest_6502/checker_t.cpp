#include "stdafx.h"
#include "checker_t.h"

void checker_t::addActualCycle(const uint16_t address, const uint8_t value, const std::string_view action) {
    m_actualCycles.push_back({ address, value, action });
}

void checker_t::addActualCycle(const EightBit::register16_t address, const uint8_t value, const std::string_view action) {
    addActualCycle(address.word, value, action);
}

void checker_t::addActualReadCycle(const EightBit::register16_t address, const uint8_t value) {
    addActualCycle(address, value, m_read_action);
}

void checker_t::addActualWriteCycle(const EightBit::register16_t address, const uint8_t value) {
    addActualCycle(address, value, m_write_action);
}

void checker_t::dumpCycles(const std::string which, const actual_cycles_t& events) {
    m_messages.push_back(which);
    dumpCycles(events);
}

void checker_t::dumpCycles(const actual_cycles_t& cycles) {
    for (const auto& cycle : cycles)
        dumpCycle(cycle);
}

void checker_t::dumpCycle(const actual_cycle_t& cycle) {
    dumpCycle(std::get<0>(cycle), std::get<1>(cycle), std::get<2>(cycle));
}

void checker_t::dumpCycles(const std::string which, const cycles_t events) {
    m_messages.push_back(which);
    dumpCycles(events);
}

void checker_t::dumpCycles(const cycles_t cycles) {
    for (const auto cycle : cycles)
        dumpCycle(cycle_t(cycle));
}

void checker_t::dumpCycle(const cycle_t cycle) {
    dumpCycle(cycle.address(), cycle.value(), cycle.action());
}

void checker_t::raise(std::string_view what, const uint16_t expected, const uint16_t actual) {
    os()
        << std::setw(2) << std::setfill(' ')
        << what
        << std::setw(4) << std::setfill('0')
        << ": expected: " << (int)expected
        << ", actual: " << (int)actual;
    pushCurrentMessage();
}

void checker_t::raise(std::string_view what, const uint8_t expected, const uint8_t actual) {
    os()
        << std::setw(2) << std::setfill(' ')
        << what
        << std::setfill('0')
        << ": expected: " << (int)expected
        << " (" << EightBit::Disassembly::dump_Flags(expected) << ")"
        << ", actual: " << (int)actual
        << " (" << EightBit::Disassembly::dump_Flags(actual) << ")";
    pushCurrentMessage();
}

void checker_t::raise(std::string_view what, const std::string_view expected, const std::string_view actual) {
    os()
        << std::setw(0) << std::setfill(' ')
        << what
        << ": expected: " << expected
        << ", actual: " << actual;
    pushCurrentMessage();
}

bool checker_t::check(std::string_view what, const uint16_t address, const uint8_t expected, const uint8_t actual) {
    const auto success = actual == expected;
    if (!success) {
        os() << what << ": " << std::setw(4) << std::setfill('0') << (int)address;
        raise(os().str(), expected, actual);
    }
    return success;
}

checker_t::checker_t(TestRunner& runner)
: m_runner(runner) {}

void checker_t::initialiseState(const test_t test) {

    auto& cpu = runner().CPU();
    auto& ram = runner().RAM();

    const auto initial = test.initial();

    cpu.PC().word = initial.pc();
    cpu.S() = initial.s();
    cpu.A() = initial.a();
    cpu.X() = initial.x();
    cpu.Y() = initial.y();
    cpu.P() = initial.p();
    for (const auto entry : initial.ram()) {
        const byte_t byte{ entry };
		const auto address = byte.address();
		const auto value = byte.value();
        ram.poke(address, value);
    }
}

void checker_t::initialise() {

    auto& bus = runner();

    bus.ReadByte.connect([this](EightBit::EventArgs&) {
        addActualReadCycle(runner().ADDRESS(), runner().DATA());
    });

    bus.WrittenByte.connect([this](EightBit::EventArgs&) {
        addActualWriteCycle(runner().ADDRESS(), runner().DATA());
    });

    os() << std::hex << std::uppercase;
}

//
bool checker_t::checkState(test_t test) {

    auto& cpu = runner().CPU();
    auto& ram = runner().RAM();

    const auto& expected_cycles = test.cycles();
    const auto& actual_cycles = m_actualCycles;

    size_t actual_idx = 0;
    for (const auto expected_cycle : expected_cycles) {

        if (actual_idx >= actual_cycles.size()) {
            m_cycle_count_mismatch = true;
            return false; // more expected cycles than actual
        }

        const cycle_t expected{ expected_cycle };
        const auto& actual = actual_cycles[actual_idx++];

        const auto expected_address = expected.address();
        const auto actual_address = std::get<0>(actual);
        check("Cycle address", expected_address, actual_address);

        const auto expected_value = expected.value();
        const auto actual_value = std::get<1>(actual);
        check("Cycle value", expected_value, actual_value);

        const auto expected_action = expected.action();
        const auto actual_action = std::get<2>(actual);
        check("Cycle action", expected_action.value_unsafe(), actual_action);
    }

    if (actual_idx < actual_cycles.size()) {
        m_cycle_count_mismatch = true;
        return false; // less expected cycles than actual
    }

    if (!m_messages.empty())
        return false;

    const auto final = test.final();
    const auto pc_good = check("PC", final.pc(), cpu.PC().word);
    const auto s_good = check("S", final.s(), cpu.S());
    const auto a_good = check("A", final.a(), cpu.A());
    const auto x_good = check("X", final.x(), cpu.X());
    const auto y_good = check("Y", final.y(), cpu.Y());
    const auto p_good = check("P", final.p(), cpu.P());

    bool ram_problem = false;
    for (const auto entry : final.ram()) {
        const auto byte = byte_t{ entry };
        const auto address = byte.address();
        const auto value = byte.value();
        if (!check("RAM", address, value, ram.peek(address))) {
			ram_problem = true;
            break;
        }
    }

    return
        pc_good && s_good
        && a_good && x_good && y_good && p_good
        && !ram_problem;
}
//
void checker_t::pushCurrentMessage() {
    m_messages.push_back(os().str());
    os().str("");
}

std::string checker_t::disassemble(uint16_t address) {
    return m_disassembler.disassemble(address);
}

void checker_t::add_disassembly(uint16_t address) {
    try {
        os() << disassemble(address);
    }
    catch (const std::domain_error& error) {
        os() << "Disassembly problem: " << error.what();
    }
    pushCurrentMessage();
}

void checker_t::check(test_t test) {

    auto& cpu = runner().CPU();

    m_messages.clear();
    m_actualCycles.clear();

    runner().raisePOWER();
    initialiseState(test);
    const auto pc = cpu.PC().word;

    m_cycles = cpu.step();
    runner().lowerPOWER();

    m_valid = checkState(test);

    if (unimplemented()) {
        m_messages.push_back("Unimplemented");
        return;
    }

    if (invalid() && implemented()) {

        add_disassembly(pc);

        const auto final = test.final();
        raise("PC", final.pc(), cpu.PC().word);
        raise("S", final.s(), cpu.S());
        raise("A", final.a(), cpu.A());
        raise("X", final.x(), cpu.X());
        raise("Y", final.y(), cpu.Y());
        raise("P", final.p(), cpu.P());

        os()
            << std::dec << std::setfill(' ')
            << "Stepped cycles: " << cycles()
            << ", expected events: " << test.cycles().size()
            << ", actual events: " << m_actualCycles.size();
        pushCurrentMessage();

        dumpCycles("-- Expected cycles", test.cycles());
        dumpCycles("-- Actual cycles", m_actualCycles);
    }
}
