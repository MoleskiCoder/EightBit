#include "stdafx.h"
#include "FuseTestRunner.h"
#include "Disassembler.h"

Fuse::TestRunner::TestRunner(const Test& test, const ExpectedTestResult& expected)
: m_test(test),
  m_expected(expected),
  m_cpu(m_bus),
  m_failed(false),
  m_unimplemented(false) {
	m_bus.clear();
	m_cpu.initialise();
}

//

void Fuse::TestRunner::initialise() {
	m_bus.disableBootRom();
	m_bus.disableGameRom();
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
			m_bus.poke(address + i, bytes[i]);
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

	auto sp = m_cpu.SP().word == expectedRegisters[Fuse::RegisterState::SP].word;
	auto pc = m_cpu.PC().word == expectedRegisters[Fuse::RegisterState::PC].word;

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
