#include "stdafx.h"
#include "../inc/IntelProcessor.h"

std::array<int, 8> EightBit::IntelProcessor::m_halfCarryTableAdd = { { 0, 0, 1, 0, 1, 0, 1, 1 } };
std::array<int, 8> EightBit::IntelProcessor::m_halfCarryTableSub = { { 0, 1, 1, 1, 0, 0, 0, 1 } };

EightBit::IntelProcessor::IntelProcessor(Bus& bus)
: LittleEndianProcessor(bus) {
	for (int i = 0; i < 0x100; ++i)
		m_decodedOpcodes.at(i) = i;

	LoweredHALT.connect([this](EventArgs) noexcept { --PC(); });
	RaisedHALT.connect([this](EventArgs) noexcept { ++PC(); });

	RaisedPOWER.connect([this](EventArgs) {
		PC() = SP() = Mask16;
		resetWorkingRegisters();
		raiseHALT();
	});
}

void EightBit::IntelProcessor::resetWorkingRegisters() {
	AF() = BC() = DE() = HL() = Mask16;
}

DEFINE_PIN_LEVEL_CHANGERS(HALT, IntelProcessor);

void EightBit::IntelProcessor::handleRESET() {
	Processor::handleRESET();
	PC() = 0;
}

void EightBit::IntelProcessor::push(const uint8_t value) {
	memoryWrite(--SP(), value);
}

uint8_t EightBit::IntelProcessor::pop() {
	return memoryRead(SP()++);
}

EightBit::register16_t EightBit::IntelProcessor::getWord() {
	const auto returned = LittleEndianProcessor::getWord();
	MEMPTR() = BUS().ADDRESS();
	return returned;
}

void EightBit::IntelProcessor::setWord(const register16_t value) {
	LittleEndianProcessor::setWord(value);
	MEMPTR() = BUS().ADDRESS();
}

void EightBit::IntelProcessor::restart(const uint8_t address) {
	call(MEMPTR() = { address, 0 });
}

int EightBit::IntelProcessor::callConditional(const int condition) {
	MEMPTR() = fetchWord();
	if (condition)
		call(MEMPTR());
	return condition;
}

int EightBit::IntelProcessor::jumpConditional(const int condition) {
	MEMPTR() = fetchWord();
	if (condition)
		jump(MEMPTR());
	return condition;
}

int EightBit::IntelProcessor::returnConditional(const int condition) {
	if (condition)
		ret();
	return condition;
}

void EightBit::IntelProcessor::jr(const int8_t offset) {
	jump(MEMPTR() = PC() + offset);
}

int EightBit::IntelProcessor::jrConditional(const int condition) {
	const auto offsetAddress = PC()++;
	if (condition) {
		const auto offset = memoryRead(offsetAddress);
		jr(offset);
	}
	return condition;
}

void EightBit::IntelProcessor::ret() {
	Processor::ret();
	MEMPTR() = PC();
}

bool EightBit::IntelProcessor::operator==(const EightBit::IntelProcessor& rhs) const {
	return
		Processor::operator==(rhs)
		&& HALT() == rhs.HALT()
		&& MEMPTR() == rhs.MEMPTR()
		&& SP() == rhs.SP()
		&& AF() == rhs.AF()
		&& BC() == rhs.BC()
		&& DE() == rhs.DE()
		&& HL() == rhs.HL();
}
