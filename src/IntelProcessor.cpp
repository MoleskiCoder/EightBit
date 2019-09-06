#include "stdafx.h"
#include "IntelProcessor.h"

EightBit::IntelProcessor::IntelProcessor(Bus& bus)
: LittleEndianProcessor(bus) {
	for (int i = 0; i < 0x100; ++i)
		m_decodedOpcodes[i] = i;

	LoweredHALT.connect([this](EventArgs) { --PC(); });
	RaisedHALT.connect([this](EventArgs) { ++PC(); });

	RaisedPOWER.connect([this](EventArgs) {
		raiseHALT();
		SP() = AF() = BC() = DE() = HL() = Mask16;
	});
}

DEFINE_PIN_LEVEL_CHANGERS(HALT, IntelProcessor);

void EightBit::IntelProcessor::handleRESET() {
	Processor::handleRESET();
	PC() = 0;
}

void EightBit::IntelProcessor::push(const uint8_t value) {
	busWrite(--SP(), value);
}

uint8_t EightBit::IntelProcessor::pop() {
	return busRead(SP()++);
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

void EightBit::IntelProcessor::ret() {
	Processor::ret();
	MEMPTR() = PC();
}
