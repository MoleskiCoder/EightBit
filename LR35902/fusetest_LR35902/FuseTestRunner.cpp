#include "stdafx.h"
#include "FuseTestRunner.h"
#include "Disassembler.h"

Fuse::TestRunner::TestRunner(const Test& test, const ExpectedTestResult& expected)
: m_test(test),
  m_expected(expected) {
}

//

void Fuse::TestRunner::powerOn() {
	EightBit::Bus::powerOn();
	CPU().powerOn();
	initialiseRegisters();
	initialiseMemory();
}

void Fuse::TestRunner::powerOff() {
	CPU().powerOff();
	EightBit::Bus::powerOff();
}

void Fuse::TestRunner::initialise() {
	disableGameRom();
}

void Fuse::TestRunner::initialiseRegisters() {

	const auto& testState = m_test.registerState;
	const auto& inputRegisters = testState.registers;

	CPU().AF() = inputRegisters[Fuse::RegisterState::AF];
	CPU().BC() = inputRegisters[Fuse::RegisterState::BC];
	CPU().DE() = inputRegisters[Fuse::RegisterState::DE];
	CPU().HL() = inputRegisters[Fuse::RegisterState::HL];

	CPU().SP() = inputRegisters[Fuse::RegisterState::SP];
	CPU().PC() = inputRegisters[Fuse::RegisterState::PC];
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

	auto af = CPU().AF() == expectedRegisters[Fuse::RegisterState::AF];
	auto bc = CPU().BC() == expectedRegisters[Fuse::RegisterState::BC];
	auto de = CPU().DE() == expectedRegisters[Fuse::RegisterState::DE];
	auto hl = CPU().HL() == expectedRegisters[Fuse::RegisterState::HL];

	auto sp = CPU().SP() == expectedRegisters[Fuse::RegisterState::SP];
	auto pc = CPU().PC() == expectedRegisters[Fuse::RegisterState::PC];

	auto success =
		af && bc && de && hl
		&& sp && pc;

	if (!success) {
		m_failed = true;
		std::cerr << "**** Failed test (Register): " << m_test.description << std::endl;

		if (!af) {
			auto expectedA = expectedRegisters[Fuse::RegisterState::AF].high;
			auto gotA = CPU().A();
			if (expectedA != gotA)
				dumpDifference("A", gotA, expectedA);

			auto expectedF = expectedRegisters[Fuse::RegisterState::AF].low;
			auto gotF = CPU().F();
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
			auto actualWord = CPU().BC();
			dumpDifference("B", "C", actualWord, expectedWord);
		}

		if (!de) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::DE];
			auto actualWord = CPU().DE();
			dumpDifference("D", "E", actualWord, expectedWord);
		}

		if (!hl) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::HL];
			auto actualWord = CPU().HL();
			dumpDifference("H", "L", actualWord, expectedWord);
		}

		if (!sp) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::SP];
			auto actualWord = CPU().SP();
			dumpDifference("SPH", "SPL", actualWord, expectedWord);
		}

		if (!pc) {
			auto expectedWord = expectedRegisters[Fuse::RegisterState::PC];
			auto actualWord = CPU().PC();
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
			auto actual = peek(address);
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

	powerOn();
	auto allowedCycles = m_test.registerState.tstates;
	try {
		CPU().run(allowedCycles);
		check();
	} catch (std::logic_error& error) {
		m_unimplemented = true;
		std::cerr << "**** Error: " << error.what() << std::endl;
	}
}
