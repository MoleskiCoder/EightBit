#include "stdafx.h"
#include "checker_t.h"

std::set<uint8_t> checker_t::m_undocumented_opcodes;
bool checker_t::m_undocumented_opcodes_initialised = false;

void checker_t::addActualCycle(const uint16_t address, const uint8_t value, const std::string action) {
    m_actualCycles.push_back({ address, value, action });
}

void checker_t::addActualCycle(const EightBit::register16_t address, const uint8_t value, const std::string action) {
    addActualCycle(address.word, value, action);
}

void checker_t::addActualReadCycle(const EightBit::register16_t address, const uint8_t value) {
    addActualCycle(address, value, "read");
}

void checker_t::addActualWriteCycle(const EightBit::register16_t address, const uint8_t value) {
    addActualCycle(address, value, "write");
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
        auto data = entry.begin();
        const int64_t address = (*data).get_int64();
        const int64_t value = (*++data).get_int64();
        ram.poke((uint16_t)address, (uint8_t)value);
    }
}

void checker_t::initialise() {

    seedUndocumentedOpcodes();

    auto& bus = runner();

    bus.ReadByte.connect([this, &bus](EightBit::EventArgs&) {
        addActualReadCycle(bus.ADDRESS(), bus.DATA());
    });

    bus.WrittenByte.connect([this, &bus](EightBit::EventArgs&) {
        addActualWriteCycle(bus.ADDRESS(), bus.DATA());
    });

    os() << std::hex << std::uppercase;
}

//
bool checker_t::checkState(test_t test) {

    auto& cpu = runner().CPU();
    auto& ram = runner().RAM();

    const auto expected_cycles = test.cycles();
    const auto& actual_cycles = m_actualCycles;
    m_cycle_count_mismatch = expected_cycles.size() != actual_cycles.size();
    if (m_cycle_count_mismatch)
        return false;

    size_t actual_idx = 0;
    for (const auto expected_cycle : expected_cycles) {

        auto expected_data = expected_cycle.begin();
        const auto& actual = actual_cycles.at(actual_idx++);   // actual could be less than expected

        const int64_t expected_address = (*expected_data).get_int64();
        const auto actual_address = std::get<0>(actual);
        check("Cycle address", (uint16_t)expected_address, actual_address);

        const int64_t expected_value = (*++expected_data).get_int64();
        const auto actual_value = std::get<1>(actual);
        check("Cycle value", (uint8_t)expected_value, actual_value);

        const std::string_view expected_action = (*++expected_data).get_string();
        const auto& actual_action = std::get<2>(actual);
        check("Cycle action", expected_action, std::string_view(actual_action));
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
        auto data = entry.begin();
        const int64_t address = (*data).get_int64();
        const int64_t value = (*++data).get_int64();
        const auto ram_good = check("RAM", address, value, ram.peek(address));
        if (!ram_good && !ram_problem)
            ram_problem = true;
    }

    return pc_good && s_good && a_good && x_good && y_good && p_good && !ram_problem;
}
//
void checker_t::pushCurrentMessage() {
    m_messages.push_back(os().str());
    os().str("");
}

void checker_t::disassemble(uint16_t address) {
    try {
        os() << m_disassembler.disassemble(address);
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
    const auto start_opcode = runner().peek(pc);

    m_undocumented = m_undocumented_opcodes.contains(start_opcode);
    if (undocumented()) {
        m_valid = false;
        m_messages.push_back("Undocumented");
        return;
    }

    m_cycles = cpu.step();
    runner().lowerPOWER();

    m_valid = checkState(test);

    if (unimplemented()) {
        m_messages.push_back("Unimplemented");
        return;
    }

    if (invalid() && implemented()) {

        disassemble(pc);

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

void checker_t::seedUndocumentedOpcodes() {
    if (m_undocumented_opcodes_initialised) return;
    m_undocumented_opcodes = {
        0x02, 0x03, 0x04, 0x07, 0x0b, 0x0c, 0x0f,
        0x12, 0x13, 0x14, 0x17, 0x1a, 0x1b, 0x1c, 0x1f,
        0x22, 0x23, 0x27, 0x2b, 0x2f,
        0x32, 0x33, 0x34, 0x37, 0x3a, 0x3b, 0x3c, 0x3f,
        0x42, 0x43, 0x44, 0x47, 0x4b, 0x4f,
        0x52, 0x53, 0x54, 0x57, 0x5a, 0x5b, 0x5c, 0x5f,
        0x62, 0x63, 0x64, 0x67, 0x6b, 0x6f,
        0x72, 0x73, 0x74, 0x77, 0x7a, 0x7b, 0x7c, 0x7f,
        0x80, 0x82, 0x83, 0x87, 0x89, 0x8b, 0x8f,
        0x92, 0x93, 0x97, 0x9b, 0x9c, 0x9e, 0x9f,
        0xa3, 0xa7, 0xab, 0xaf,
        0xb2, 0xb3, 0xb7, 0xbb, 0xbf,
        0xc2, 0xc3, 0xc7, 0xcb, 0xcf,
        0xd2, 0xd3, 0xd4, 0xd7, 0xda, 0xdb, 0xdc, 0xdf,
        0xe2, 0xe3, 0xe7, 0xeb, 0xef,
        0xf1, 0xf2, 0xf3, 0xf4, 0xf7, 0xfa, 0xfb, 0xfc, 0xff,
    };
    m_undocumented_opcodes_initialised = true;
}
