#include "stdafx.h"
#include "FuseTestRunner.h"
#include "Disassembler.h"

Fuse::TestRunner::TestRunner(const Test& test, const ExpectedTestResult& expected)
: m_test(test),
  m_expected(expected),
  m_ram(0x10000),
  m_cpu(*this),
  m_failed(false),
  m_unimplemented(false) {
}

//

void Fuse::TestRunner::initialise() {
	m_cpu.powerOn();
	disableGameRom();
	initialiseRegisters();
	initialiseMemory();
}

void Fuse::TestRunner::initialiseRegisters() {

	const auto& testState = m_test.registerState;
	const auto& inputRegisters = testState.registers;

	m_cpu.AF() = inputRegisters[Fuse::RegisterState::AF];
	m_cpu.BC() = inputRegisters[Fuse::RegisterState::BC];
	m_cpu.DE() = inputRegisters[Fuse::RegisterState::DE];
	m_cpu.HL() = inputRegisters[Fuse::RegisterState::HL];

	m_cpu.SP() = inputRegisters[Fuse::RegisterState::SP];
	m_cpu.PC() = inputRegisters[Fuse::RegisterState::PC];
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
		<< EightBit::GameBoy::Disassembler::hex(expected)
		<< ", Got: "
		<< EightBit::GameBoy::Disassembler::hex(actual)
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

	auto sp = m_cpu.SP() == expectedRegisters[Fuse::RegisterState::SP];
	auto pc = m_cpu.PC() == expectedRegisters[Fuse::RegisterState::PC];

	auto success =
		af && bc && de && hl
		&& sp && pc;

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
					<< EightBit::GameBoy::Disassembler::flags(expectedF)
					<< ", Got: "
					<< EightBit::GameBoy::Disassembler::flags(gotF)
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
	}
}

void Fuse::TestRunner::checkMemory() {

	bool first = true;

	for (auto memoryDatum : m_expected.memoryData) {
		auto bytes = memoryDatum.bytes;
		for (int i = 0; i < bytes.size(); ++i) {
			auto expected = bytes[i];
			uint16_t address = memoryDatum.address + i;
			auto actual = m_cpu.BUS().peek(address);
			if (expected != actual) {
				m_failed = true;
				if (first) {
					first = false;
					std::cerr << "**** Failed test (Memory): " << m_test.description << std::endl;
				}
				std::cerr
					<< "**** Difference: "
					<< "Address: " << EightBit::GameBoy::Disassembler::hex(address)
					<< " Expected: " << EightBit::GameBoy::Disassembler::hex(expected)
					<< " Actual: " << EightBit::GameBoy::Disassembler::hex(actual)
					<< std::endl;
			}
		}
	}
}

void Fuse::TestRunner::run() {

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
