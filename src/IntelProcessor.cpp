#include "stdafx.h"
#include "../inc/IntelProcessor.h"

std::array<int, 8> EightBit::IntelProcessor::m_halfCarryTableAdd = { { 0, 0, 1, 0, 1, 0, 1, 1 } };
std::array<int, 8> EightBit::IntelProcessor::m_halfCarryTableSub = { { 0, 1, 1, 1, 0, 0, 0, 1 } };

EightBit::IntelProcessor::IntelProcessor(Bus& bus)
: LittleEndianProcessor(bus) {
	for (int i = 0; i < 0x100; ++i)
		m_decodedOpcodes.at(i) = i;

	RaisedPOWER.connect([this](EventArgs) {
		PC() = SP() = Mask16;
		resetWorkingRegisters();
		raiseHALT();
	});
}

EightBit::IntelProcessor::IntelProcessor(const IntelProcessor& rhs) noexcept
: LittleEndianProcessor(rhs) {

	m_sp = rhs.m_sp;
	m_memptr = rhs.m_memptr;

	HALT() = rhs.HALT();
}

void EightBit::IntelProcessor::resetWorkingRegisters() noexcept {
	AF() = BC() = DE() = HL() = Mask16;
}

DEFINE_PIN_LEVEL_CHANGERS(HALT, IntelProcessor);

void EightBit::IntelProcessor::handleRESET() {
	Processor::handleRESET();
	disableInterrupts();
	Processor::jump(0);
}

void EightBit::IntelProcessor::handleINT() {
	Processor::handleINT();
	disableInterrupts();
	raiseHALT();
}

void EightBit::IntelProcessor::push(const uint8_t value) {
	memoryWrite(--SP(), value);
}

uint8_t EightBit::IntelProcessor::pop() {
	return memoryRead(SP()++);
}


EightBit::register16_t& EightBit::IntelProcessor::incrementPC() {
	if (raised(HALT()))
		Processor::incrementPC();
	return PC();
}

uint8_t EightBit::IntelProcessor::fetchInstruction() {
	const auto data = fetchByte();
	return lowered(HALT()) ? (uint8_t)0 : data;
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
	MEMPTR() = { address, 0 };
	call();
}

void EightBit::IntelProcessor::callConditional(bool condition) {
	MEMPTR() = fetchWord();
	if (condition)
		call();
}

void EightBit::IntelProcessor::jumpConditional(bool condition) {
	MEMPTR() = fetchWord();
	if (condition)
		jump();
}

void EightBit::IntelProcessor::jumpRelativeConditional(bool condition) {
	const auto offset = fetchByte();
	if (condition)
		jumpRelative(offset);
}

void EightBit::IntelProcessor::returnConditional(bool condition) {
	if (condition)
		ret();
}

void EightBit::IntelProcessor::jumpRelative(const int8_t offset) noexcept {
	MEMPTR() = PC() + offset;
	jump();
}


void EightBit::IntelProcessor::ret() {
	LittleEndianProcessor::ret();
	MEMPTR() = PC();
}

void EightBit::IntelProcessor::jumpIndirect() {
	MEMPTR() = fetchWord();
	jump();
}

void EightBit::IntelProcessor::jump() {
	Processor::jump(MEMPTR());
}

void EightBit::IntelProcessor::callIndirect() {
	MEMPTR() = fetchWord();
	call();
}

void EightBit::IntelProcessor::call() {
	call(MEMPTR());
}

void EightBit::IntelProcessor::call(register16_t destination) {
	Processor::call(destination);
}

bool EightBit::IntelProcessor::operator==(const EightBit::IntelProcessor& rhs) const noexcept {
	return
		LittleEndianProcessor::operator==(rhs)
		&& HALT() == rhs.HALT()
		&& MEMPTR() == rhs.MEMPTR()
		&& SP() == rhs.SP()
		&& AF() == rhs.AF()
		&& BC() == rhs.BC()
		&& DE() == rhs.DE()
		&& HL() == rhs.HL();
}
