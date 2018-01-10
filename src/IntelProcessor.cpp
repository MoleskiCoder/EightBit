#include "stdafx.h"
#include "IntelProcessor.h"

EightBit::IntelProcessor::IntelProcessor(Bus& bus)
: Processor(bus) {
	for (int i = 0; i < 0x100; ++i)
		m_decodedOpcodes[i] = i;
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

void EightBit::IntelProcessor::getWord(register16_t& value) {
	value.low = getByte(MEMPTR().word++);
	BUS().ADDRESS().word++;
	value.high = getByte();
}

void EightBit::IntelProcessor::setWord(register16_t value) {
	setByte(MEMPTR().word++, value.low);
	BUS().ADDRESS().word++;
	setByte(value.high);
}
