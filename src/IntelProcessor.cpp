#include "stdafx.h"
#include "IntelProcessor.h"

EightBit::IntelProcessor::IntelProcessor(Bus& bus)
: Processor(bus) {
	for (int i = 0; i < 0x100; ++i)
		m_decodedOpcodes[i] = i;
}

void EightBit::IntelProcessor::reset() {
	Processor::reset();
	SP() = AF() = BC() = DE() = HL() = Mask16;
}

void EightBit::IntelProcessor::push(const uint8_t value) {
	BUS().write(--SP(), value);
}

uint8_t EightBit::IntelProcessor::pop() {
	return BUS().read(SP()++);
}

EightBit::register16_t EightBit::IntelProcessor::getWord() {
	const auto low = BUS().read();
	MEMPTR() = ++BUS().ADDRESS();
	const auto high = BUS().read();
	return register16_t(low, high);
}

void EightBit::IntelProcessor::setWord(const register16_t value) {
	BUS().write(value.low);
	MEMPTR() = ++BUS().ADDRESS();
	BUS().write(value.high);
}
