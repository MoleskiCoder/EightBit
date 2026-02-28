#include "stdafx.h"
#include "checker_t.h"
#include "port_t.h"

void checker_t::addActualCycle(const uint16_t address, const uint8_t value, const std::string& action) {
    m_actualCycles.push_back({ address, value, action });
}

void checker_t::addActualCycle(const EightBit::register16_t address, const uint8_t value, const std::string& action) {
    addActualCycle(address.word, value, action);
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

void checker_t::raise(const std::string& what, const uint16_t expected, const uint16_t actual) {
    os()
        << std::setw(2) << std::setfill(' ')
        << what
        << std::setw(4) << std::setfill('0')
        << ": expected: " << (int)expected
        << ", actual: " << (int)actual;
    pushCurrentMessage();
}

void checker_t::raise(const std::string& what, const uint8_t expected, const uint8_t actual) {
    os()
        << std::setw(2) << std::setfill(' ')
        << what
        << std::setfill('0')
        << ": expected: " << (int)expected
        << ", actual: " << (int)actual;
    pushCurrentMessage();
}

void checker_t::raiseFlags(const std::string& what, const uint8_t expected, const uint8_t actual) {
    os()
        << std::setw(2) << std::setfill(' ')
        << what
        << std::setfill('0')
        << ": expected: " << EightBit::Disassembler::flags(expected)
        << ", actual: " << EightBit::Disassembler::flags(actual);
    pushCurrentMessage();
}

void checker_t::raise(const std::string& what, const std::string& expected, const std::string& actual) {
    os()
        << std::setw(0) << std::setfill(' ')
        << what
        << ": expected: " << expected
        << ", actual: " << actual;
    pushCurrentMessage();
}

bool checker_t::check(const std::string& what, const uint16_t address, const uint8_t expected, const uint8_t actual) {
    const auto success = actual == expected;
    if (!success) {
        os() << what << ": " << std::setw(4) << std::setfill('0') << (int)address;
        raise(os().str(), expected, actual);
    }
    return success;
}

checker_t::checker_t(TestRunner& runner)
: m_runner(runner) {}

void checker_t::initialiseState(test_t test) {
	initialiseState(test.initial(), test.ports());
}

void checker_t::initialiseState(const state_t initial, std::optional<ports_t> ports) {

    auto& cpu = runner().CPU();

    cpu.PC() = initial.pc();
    cpu.SP() = initial.sp();

	// Alternate register set, then switch to the main set

    cpu.AF() = initial.af_();
    cpu.BC() = initial.bc_();
    cpu.DE() = initial.de_();
    cpu.HL() = initial.hl_();

    cpu.exx();
    cpu.exxAF();

	// Now we're in the main register set, so set it up

    cpu.AF() = { initial.f(), initial.a() };
    cpu.BC() = { initial.c(), initial.b() };
    cpu.DE() = { initial.e(), initial.d() };
    cpu.HL() = { initial.l(), initial.h() };

	// miscellaneous registers

	cpu.IV() = initial.i();
	cpu.REFRESH() = initial.r();

	cpu.IM() = initial.im();

	cpu.Q() = initial.q();
	
    cpu.IFF1() = initial.iff1();
	cpu.IFF2() = initial.iff2();

	cpu.MEMPTR() = initial.wz();

	cpu.IX() = initial.ix();
	cpu.IY() = initial.iy();

	auto& ram = runner().RAM();
    for (const auto entry : initial.ram()) {
        const byte_t byte{ entry };
        const auto address = byte.address();
        const auto value = byte.value();
        ram.poke(address, value);
    }

    if (!ports.has_value())
		return;

    for (const auto entry : ports.value()) {
		const port_t port{ entry };
		const auto address = port.address();
		const auto value = port.value();
		const auto type = port.type();
        if (type == "r") {
			runner().ports().writeInputPort(address, value);
		} else if (type == "w") {
            runner().ports().writeOutputPort(address, value);
        } else {
            throw std::out_of_range("Unknown port state type");
        }
	}
}

void checker_t::initialise() {

	auto& bus = runner();
	auto& cpu = bus.CPU();
    cpu.Ticked.connect([this](EightBit::EventArgs&) {

        auto& bus = runner();
        auto& cpu = bus.CPU();

        const std::string read = EightBit::Device::lowered(cpu.RD()) ? "r" : "-";
        const std::string write = EightBit::Device::lowered(cpu.WR()) ? "w" : "-";
        const std::string memory = EightBit::Device::lowered(cpu.MREQ()) ? "m" : "-";
        const std::string io = EightBit::Device::lowered(cpu.IORQ()) ? "i" : "-";

        addActualCycle(bus.ADDRESS(), bus.DATA(), read + write + memory + io);
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

		const auto maybe_expected_value = expected.value();
        if (maybe_expected_value.has_value()) {
            const auto expected_value = maybe_expected_value.value();
            const auto actual_value = std::get<1>(actual);
            check("Cycle value", expected_value, actual_value);
        }

		const auto expected_action = expected.action();
        const auto& actual_action = std::get<2>(actual);
        check("Cycle action", std::string(expected_action), actual_action);
    }

    if (actual_idx < actual_cycles.size()) {
        m_cycle_count_mismatch = true;
        return false; // less expected cycles than actual
    }

    if (!m_messages.empty())
        return false;

    const auto final = test.final();

    const auto pc_good = check("PC", final.pc(), cpu.PC().word);
    const auto sp_good = check("SP", final.sp(), cpu.SP().word);

    const auto a_good = check("A", final.a(), cpu.A());
    const auto f_good = check("F", final.f(), cpu.F());
	const auto af_good = a_good && f_good;

    const auto b_good = check("B", final.b(), cpu.B());
    const auto c_good = check("C", final.c(), cpu.C());
    const auto bc_good = b_good && c_good;

    const auto d_good = check("D", final.d(), cpu.D());
    const auto e_good = check("E", final.e(), cpu.E());
    const auto de_good = d_good && e_good;

    const auto h_good = check("H", final.h(), cpu.H());
    const auto l_good = check("L", final.l(), cpu.L());
    const auto hl_good = h_good && l_good;

    cpu.exxAF();
	cpu.exx();

    const auto af_alt_good = check("AF'", final.af_(), cpu.AF().word);
    const auto bc_alt_good = check("BC'", final.bc_(), cpu.BC().word);
    const auto de_alt_good = check("DE'", final.de_(), cpu.DE().word);
    const auto hl_alt_good = check("DE'", final.de_(), cpu.DE().word);

	const auto i_good = check("I", final.i(), cpu.IV());
	const auto r_good = check("R", final.r(), (uint8_t)cpu.REFRESH());

	const auto im_good = check("IM", final.im(), (uint8_t)cpu.IM());

    const auto q_good = check("Q", final.im(), (uint8_t)cpu.IM());

    const auto iff1_good = check("IFF1", final.iff1(), (uint8_t)cpu.IFF1());
    const auto iff2_good = check("IFF2", final.iff2(), (uint8_t)cpu.IFF2());
	const auto iff_good = iff1_good && iff2_good;

    const auto memptr_good = check("MEMPTR", final.wz(), cpu.MEMPTR().word);

    const auto ix_good = check("IX", final.ix(), cpu.IX().word);
    const auto iy_good = check("IY", final.iy(), cpu.IY().word);

    bool ram_problem = false;
    for (const auto entry : final.ram()) {
        const auto byte = byte_t{ entry };
        const auto address = byte.address();
        const auto value = byte.value();
        if (!check("RAM", address, value, ram.peek(address)))
            ram_problem = true;
    }

    return
        pc_good && sp_good
        && af_good && bc_good && de_good && hl_good
        && af_alt_good && bc_alt_good && de_alt_good && hl_alt_good
		&& i_good && r_good
        && im_good
        && q_good
        && iff_good
        && memptr_good
        && ix_good && iy_good
        && !ram_problem;
}
//
void checker_t::pushCurrentMessage() {
    m_messages.push_back(os().str());
    os().str("");
}

std::string checker_t::disassemble(uint16_t address) {
    return m_disassembler.disassemble(runner().CPU(), address);
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
        raise("SP", final.sp(), cpu.SP().word);

        raise("A", final.a(), cpu.A());
        raiseFlags("F", final.a(), cpu.A());
        raise("B", final.b(), cpu.B());
        raise("C", final.c(), cpu.C());
        raise("D", final.d(), cpu.D());
        raise("E", final.e(), cpu.E());
        raise("H", final.h(), cpu.H());
        raise("L", final.l(), cpu.L());

        cpu.exx();
        cpu.exxAF();

        raise("'AF", final.af_(), cpu.AF().word);
        raise("'BC", final.bc_(), cpu.BC().word);
        raise("'DE", final.de_(), cpu.DE().word);
        raise("'HL", final.hl_(), cpu.HL().word);

        raise("I", final.i(), cpu.IV());
        raise("R", final.r(), cpu.REFRESH());

        raise("IM", final.im(), (uint8_t)cpu.IM());

        raise("Q", final.q(), cpu.Q());

        raise("IFF1", final.iff1(), (uint8_t)cpu.IFF1());
        raise("IFF2", final.iff2(), (uint8_t)cpu.IFF2());

        raise("WZ", final.wz(), cpu.MEMPTR().word);

        raise("IX", final.ix(), cpu.IX().word);
        raise("IY", final.iy(), cpu.IY().word);

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
