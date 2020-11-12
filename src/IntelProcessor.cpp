#include "stdafx.h"
#include "IntelProcessor.h"

EightBit::IntelProcessor::IntelProcessor(Bus& bus)
: LittleEndianProcessor(bus) {
	for (int i = 0; i < 0x100; ++i)
		m_decodedOpcodes.at(i) = i;

	LoweredHALT.connect([this](EventArgs) noexcept { --PC(); });
	RaisedHALT.connect([this](EventArgs) noexcept { ++PC(); });

	RaisedPOWER.connect([this](EventArgs) {
		PC() = SP() = AF() = BC() = DE() = HL() = Mask16;
		raiseHALT();
	});
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
