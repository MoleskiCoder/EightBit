#include "stdafx.h"
#include "Processor.h"

EightBit::Processor::Processor(Bus& bus)
: m_bus(bus) {
}

void EightBit::Processor::reset() {
	if (lowered(POWER()))
		throw std::logic_error("POWER cannot be low");
	raise(INT());
	raise(NMI());
	raise(RESET());
	PC().word = MEMPTR().word = 0;
}

int EightBit::Processor::run(int limit) {
	int current = 0;
	while (LIKELY(powered()) && current < limit) {
		current += singleStep();
	}
	return current;
}

int EightBit::Processor::singleStep() {
	if (UNLIKELY(lowered(RESET())))
		reset();
	return step();
}

uint8_t EightBit::Processor::fetchByte() {
	return getByte(PC().word++);
}

void EightBit::Processor::fetchWord(register16_t& output) {
	output.low = fetchByte();
	output.high = fetchByte();
}
