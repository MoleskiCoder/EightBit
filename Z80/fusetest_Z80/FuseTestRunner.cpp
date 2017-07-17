#include "stdafx.h"
#include "FuseTestRunner.h"
#include "Disassembler.h"

Fuse::TestRunner::TestRunner(const Test& test, const ExpectedTestResult& expected)
: m_test(test),
  m_expected(expected),
  m_memory(0xffff),
  m_cpu(m_memory, m_ports),
  m_failed(false),
  m_unimplemented(false) {
	m_memory.clear();
	m_cpu.initialise();
}

//

void Fuse::TestRunner::initialise() {
	initialiseRegisters();
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
		for (int i = 0; i < bytes.size(); ++i) {
			m_memory.ADDRESS().word = address + i;
			m_memory.reference() = bytes[i];
		}
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

	auto af = m_cpu.AF().word == expectedRegisters[Fuse::RegisterState::AF].word;
	auto bc = m_cpu.BC().word == expectedRegisters[Fuse::RegisterState::BC].word;
	auto de = m_cpu.DE().word == expectedRegisters[Fuse::RegisterState::DE].word;
	auto hl = m_cpu.HL().word == expectedRegisters[Fuse::RegisterState::HL].word;
	m_cpu.exx();
	m_cpu.exxAF();
	auto af_ = m_cpu.AF().word == expectedRegisters[Fuse::RegisterState::AF_].word;
	auto bc_ = m_cpu.BC().word == expectedRegisters[Fuse::RegisterState::BC_].word;
	auto de_ = m_cpu.DE().word == expectedRegisters[Fuse::RegisterState::DE_].word;
	auto hl_ = m_cpu.HL().word == expectedRegisters[Fuse::RegisterState::HL_].word;

	auto ix = m_cpu.IX().word == expectedRegisters[Fuse::RegisterState::IX].word;
	auto iy = m_cpu.IY().word == expectedRegisters[Fuse::RegisterState::IY].word;

	auto sp = m_cpu.SP().word == expectedRegisters[Fuse::RegisterState::SP].word;
	auto pc = m_cpu.PC().word == expectedRegisters[Fuse::RegisterState::PC].word;

	auto memptr = m_cpu.MEMPTR().word == expectedRegisters[Fuse::RegisterState::MEMPTR].word;

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
			auto actual = m_cpu.getMemory().peek(address);
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

	initialise();
	auto allowedCycles = m_test.registerState.tstates;
	try {
		auto cycles = 0;
		do {
			cycles += m_cpu.step();
		} while (allowedCycles > cycles);
		check();
	}	catch (std::logic_error& error) {
		m_unimplemented = true;
		std::cerr << "**** Error: " << error.what() << std::endl;
	}
}
