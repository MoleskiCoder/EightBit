#include "stdafx.h"
#include "FuseTestRunner.h"
#include "Disassembler.h"

Fuse::TestRunner::TestRunner(const Test& test, const ExpectedTestResult& expected)
: m_test(test),
  m_expected(expected),
  m_cpu(*this, m_ports) {
}

//

void Fuse::TestRunner::raisePOWER() {
	EightBit::Bus::raisePOWER();
	m_cpu.raisePOWER();
	m_cpu.raiseRESET();
	m_cpu.raiseINT();
	m_cpu.raiseNMI();
	initialiseRegisters();
}

void Fuse::TestRunner::lowerPOWER() {
	m_cpu.lowerPOWER();
	EightBit::Bus::lowerPOWER();
}

void Fuse::TestRunner::initialise() {
	initialiseMemory();
}

void Fuse::TestRunner::initialiseRegisters() {

	const auto& testState = m_test.registerState;
	const auto& inputRegisters = testState.registers;

	m_cpu.AF() = inputRegisters[Fuse::RegisterState::AF_];
	m_cpu.BC() = inputRegisters[Fuse::RegisterState::BC_];
	m_cpu.DE() = inputRegisters[Fuse::RegisterState::DE_];
	m_cpu.HL() = inputRegisters[Fuse::RegisterState::HL_];
	m_cpu.exx();
	m_cpu.exxAF();
	m_cpu.AF() = inputRegisters[Fuse::RegisterState::AF];
	m_cpu.BC() = inputRegisters[Fuse::RegisterState::BC];
	m_cpu.DE() = inputRegisters[Fuse::RegisterState::DE];
	m_cpu.HL() = inputRegisters[Fuse::RegisterState::HL];

	m_cpu.IX() = inputRegisters[Fuse::RegisterState::IX];
	m_cpu.IY() = inputRegisters[Fuse::RegisterState::IY];

	m_cpu.SP() = inputRegisters[Fuse::RegisterState::SP];
	m_cpu.PC() = inputRegisters[Fuse::RegisterState::PC];

	m_cpu.MEMPTR() = inputRegisters[Fuse::RegisterState::MEMPTR];

	m_cpu.IV() = testState.i;
	m_cpu.REFRESH() = testState.r;
	m_cpu.IFF1() = testState.iff1;
	m_cpu.IFF2() = testState.iff2;
	m_cpu.IM() = testState.im;
}

void Fuse::TestRunner::initialiseMemory() {
	for (auto memoryDatum : m_test.memoryData) {
		auto address = memoryDatum.address;
		auto bytes = memoryDatum.bytes;
		for (int i = 0; i < bytes.size(); ++i)
			poke(address + i, bytes[i]);
	}
}

//

void Fuse::TestRunner::check() {
	checkregisters();
	checkMemory();
}

void Fuse::TestRunner::dumpDifference(const std::string& description, uint8_t actual, uint8_t expected) const {
	std::cerr
		<< "**** " << description << ", Expected: "
		<< EightBit::Disassembler::hex(expected)
		<< ", Got: "
		<< EightBit::Disassembler::hex(actual)
		<< std::endl;
}

void Fuse::TestRunner::dumpDifference(
	const std::string& highDescription,
	const std::string& lowDescription,
	EightBit::register16_t actual, EightBit::register16_t expected) const {

	auto expectedHigh = expected.high;
	auto expectedLow = expected.low;

	auto actualHigh = actual.high;
	auto actualLow = actual.low;

	if (expectedHigh != actualHigh)
		dumpDifference(highDescription, actualHigh, expectedHigh);

	if (expectedLow != actualLow)
		dumpDifference(lowDescription, actualLow, expectedLow);
}

void Fuse::TestRunner::checkregisters() {

	const auto& expectedState = m_expected.registerState;
	const auto& expectedRegisters = expectedState.registers;

	auto af = m_cpu.AF() == expectedRegisters[Fuse::RegisterState::AF];
	auto bc = m_cpu.BC() == expectedRegisters[Fuse::RegisterState::BC];
	auto de = m_cpu.DE() == expectedRegisters[Fuse::RegisterState::DE];
	auto hl = m_cpu.HL() == expectedRegisters[Fuse::RegisterState::HL];
	m_cpu.exx();
	m_cpu.exxAF();
	auto af_ = m_cpu.AF() == expectedRegisters[Fuse::RegisterState::AF_];
	auto bc_ = m_cpu.BC() == expectedRegisters[Fuse::RegisterState::BC_];
	auto de_ = m_cpu.DE() == expectedRegisters[Fuse::RegisterState::DE_];
	auto hl_ = m_cpu.HL() == expectedRegisters[Fuse::RegisterState::HL_];

	auto ix = m_cpu.IX() == expectedRegisters[Fuse::RegisterState::IX];
	auto iy = m_cpu.IY() == expectedRegisters[Fuse::RegisterState::IY];

	auto sp = m_cpu.SP() == expectedRegisters[Fuse::RegisterState::SP];
	auto pc = m_cpu.PC() == expectedRegisters[Fuse::RegisterState::PC];

	auto memptr = m_cpu.MEMPTR() == expectedRegisters[Fuse::RegisterState::MEMPTR];

	auto iv = m_cpu.IV() == expectedState.i;
	auto refresh = m_cpu.REFRESH() == expectedState.r;
	auto iff1 = m_cpu.IFF1() == expectedState.iff1;
	auto iff2 = m_cpu.IFF2() == expectedState.iff2;
	auto im = m_cpu.IM() == expectedState.im;

	// And back again, so the following works as expected...
	m_cpu.exx();
	m_cpu.exxAF();

	auto success =
		af && bc && de && hl
		&& af_ && bc_ && de_ && hl_
		&& ix && iy
		&& sp && pc
		&& iv && refresh
		&& iff1 && iff2
		&& im
		&& memptr;

	if (!success) {
		m_failed = true;
		std::cerr << "**** Failed test (Register): " << m_test.description << std::endl;

		if (!af) {
			auto expectedA = expectedRegisters[Fuse::RegisterState::AF].high;
			auto gotA = m_cpu.A();
			if (expectedA != gotA)
				dumpDifference("A", gotA, expectedA);

			auto expectedF = expectedRegisters[Fuse::RegisterState::AF].low;
			auto gotF = m_cpu.F();
			if (expectedF != gotF) {
				std::cerr
					<< "**** F, Expected: "
					<< EightBit::Disassembler::flags(expectedF)
					<< ", Got: "
					<< EightBit::Disassembler::flags(gotF)
					<< std::endl;
			}
		}

		if (!bc) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::BC];
			auto actualWord = m_cpu.BC();
			dumpDifference("B", "C", actualWord, expectedWord);
		}

		if (!de) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::DE];
			auto actualWord = m_cpu.DE();
			dumpDifference("D", "E", actualWord, expectedWord);
		}

		if (!hl) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::HL];
			auto actualWord = m_cpu.HL();
			dumpDifference("H", "L", actualWord, expectedWord);
		}

		if (!ix) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::IX];
			auto actualWord = m_cpu.IX();
			dumpDifference("IXH", "IXL", actualWord, expectedWord);
		}

		if (!iy) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::IY];
			auto actualWord = m_cpu.IY();
			dumpDifference("IYH", "IYL", actualWord, expectedWord);
		}

		if (!sp) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::SP];
			auto actualWord = m_cpu.SP();
			dumpDifference("SPH", "SPL", actualWord, expectedWord);
		}

		if (!pc) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::PC];
			auto actualWord = m_cpu.PC();
			dumpDifference("PCH", "PCL", actualWord, expectedWord);
		}

		if (!memptr) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::MEMPTR];
			auto actualWord = m_cpu.MEMPTR();
			dumpDifference("MEMPTRH", "MEMPTRL", actualWord, expectedWord);
		}

		m_cpu.exxAF();
		m_cpu.exx();

		if (!af_) {
			auto expectedA_ = expectedRegisters[Fuse::RegisterState::AF_].high;
			auto gotA_ = m_cpu.A();
			if (expectedA_ != gotA_)
				dumpDifference("A'", gotA_, expectedA_);

			auto expectedF_ = expectedRegisters[Fuse::RegisterState::AF_].low;
			auto gotF_ = m_cpu.F();
			if (expectedF_ != gotF_) {
				std::cerr
					<< "**** F', Expected: "
					<< EightBit::Disassembler::flags(expectedF_)
					<< ", Got: "
					<< EightBit::Disassembler::flags(gotF_)
					<< std::endl;
			}
		}

		if (!bc_) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::BC_];
			auto actualWord = m_cpu.BC();
			dumpDifference("B'", "C'", actualWord, expectedWord);
		}

		if (!de_) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::DE_];
			auto actualWord = m_cpu.DE();
			dumpDifference("D'", "E'", actualWord, expectedWord);
		}

		if (!hl_) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::HL_];
			auto actualWord = m_cpu.HL();
			dumpDifference("H'", "L'", actualWord, expectedWord);
		}

		if (!iv) {
			std::cerr
				<< "**** IV, Expected: "
				<< EightBit::Disassembler::hex((uint8_t)expectedState.i)
				<< ", Got: "
				<< EightBit::Disassembler::hex(m_cpu.IV())
				<< std::endl;
		}

		if (!refresh) {
			std::cerr
				<< "**** R, Expected: "
				<< EightBit::Disassembler::hex((uint8_t)expectedState.r)
				<< ", Got: "
				<< EightBit::Disassembler::hex((uint8_t)m_cpu.REFRESH())
				<< std::endl;
		}

		if (!iff1) {
			std::cerr
				<< "**** IFF1, Expected: "
				<< (bool)expectedState.iff1
				<< ", Got: "
				<< m_cpu.IFF1()
				<< std::endl;
		}

		if (!iff2) {
			std::cerr
				<< "**** IFF2, Expected: "
				<< (bool)expectedState.iff2
				<< ", Got: "
				<< m_cpu.IFF2()
				<< std::endl;
		}

		if (!im) {
			std::cerr
				<< "**** IM, Expected: "
				<< expectedState.im
				<< ", Got: "
				<< m_cpu.IM()
				<< std::endl;
		}
	}
}

void Fuse::TestRunner::checkMemory() {

	bool first = true;

	for (auto memoryDatum : m_expected.memoryData) {
		auto bytes = memoryDatum.bytes;
		for (int i = 0; i < bytes.size(); ++i) {
			auto expected = bytes[i];
			uint16_t address = memoryDatum.address + i;
			auto actual = peek(address);
			if (expected != actual) {
				m_failed = true;
				if (first) {
					first = false;
					std::cerr << "**** Failed test (Memory): " << m_test.description << std::endl;
				}
				std::cerr
					<< "**** Difference: "
					<< "Address: " << EightBit::Disassembler::hex(address)
					<< " Expected: " << EightBit::Disassembler::hex(expected)
					<< " Actual: " << EightBit::Disassembler::hex(actual)
					<< std::endl;
			}
		}
	}
}

void Fuse::TestRunner::run() {
	raisePOWER();
	initialise();
	auto allowedCycles = m_test.registerState.tstates;
	try {
		m_cpu.run(allowedCycles);
		check();
	} catch (std::logic_error& error) {
		m_unimplemented = true;
		std::cerr << "**** Error: " << error.what() << std::endl;
	}
}
