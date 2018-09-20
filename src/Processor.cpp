#include "stdafx.h"
#include "Processor.h"

EightBit::Processor::Processor(Bus& bus)
: m_bus(bus) {
}

void EightBit::Processor::powerOn() {
	raise(RESET());
	raise(HALT());
	raise(INT());
	raise(POWER());
}

void EightBit::Processor::handleRESET() {
	raise(RESET());
	PC() = 0;
}

void EightBit::Processor::handleINT() {
	raise(INT());
}

void EightBit::Processor::handleIRQ() {
	raise(IRQ());
}

int EightBit::Processor::run(const int limit) {
	int current = 0;
	while (LIKELY(powered() && (current < limit)))
		current += step();
	return current;
}

// http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
int8_t EightBit::Processor::signExtend(const int b, uint8_t x) {
	const uint8_t m = 1 << (b - 1); // mask can be pre-computed if b is fixed
	x = x & ((1 << b) - 1);  // (Skip this if bits in x above position b are already zero.)
	const auto result = (x ^ m) - m;
	return result;
}
