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
	PC() = 0;
}

int EightBit::Processor::run(const int limit) {
	int current = 0;
	while (LIKELY(powered()) && current < limit)
		current += singleStep();
	return current;
}

int EightBit::Processor::singleStep() {
	if (UNLIKELY(lowered(RESET())))
		reset();
	return step();
}
