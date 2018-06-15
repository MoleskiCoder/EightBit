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

void EightBit::IntelProcessor::push(const uint8_t value) {
	BUS().write(--SP().word, value);
}

uint8_t EightBit::IntelProcessor::pop() {
	return BUS().read(SP().word++);
}

EightBit::register16_t EightBit::IntelProcessor::getWord() {
	BUS().ADDRESS().word = MEMPTR().word++;
	const auto low = BUS().read();
	++BUS().ADDRESS().word;
	const auto high = BUS().read();
	return register16_t(low, high);
}

void EightBit::IntelProcessor::setWord(const register16_t value) {
	BUS().write(MEMPTR().word++, value.low);
	BUS().write(++BUS().ADDRESS().word, value.high);
}
