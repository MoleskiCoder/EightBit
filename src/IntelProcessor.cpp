#include "stdafx.h"
#include "IntelProcessor.h"

EightBit::IntelProcessor::IntelProcessor(Bus& bus)
: Processor(bus) {
}

void EightBit::IntelProcessor::initialise() {
	Processor::initialise();
	for (int i = 0; i < 0x100; ++i) {
		m_decodedOpcodes[i] = i;
	}
}

void EightBit::IntelProcessor::reset() {
	Processor::reset();
	SP().word = AF().word = BC().word = DE().word = HL().word = Mask16;
}

void EightBit::IntelProcessor::push(uint8_t value) {
	setByte(--SP().word, value);
}

uint8_t EightBit::IntelProcessor::pop() {
	return getByte(SP().word++);
}

void EightBit::IntelProcessor::getWordViaMemptr(register16_t& value) {
	memptrReference();
	value.low = getByte();
	BUS().ADDRESS().word++;
	value.high = getByte();
}

void EightBit::IntelProcessor::setWordViaMemptr(register16_t value) {
	memptrReference();
	setByte(value.low);
	BUS().ADDRESS().word++;
	setByte(value.high);
}
