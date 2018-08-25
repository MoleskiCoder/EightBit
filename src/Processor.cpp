#include "stdafx.h"
#include "Processor.h"

EightBit::Processor::Processor(Bus& bus)
: m_bus(bus) {
}

void EightBit::Processor::powerOn() {
	raise(RESET());
	raise(HALT());
	raise(INT());
	raise(NMI());
	raise(POWER());
}

void EightBit::Processor::handleRESET() {
	raise(RESET());
	PC() = 0;
}

void EightBit::Processor::handleNMI() {
	raise(NMI());
}

void EightBit::Processor::handleINT() {
	raise(INT());
}

int EightBit::Processor::run(const int limit) {
	int current = 0;
	while (LIKELY(powered()) && current < limit)
		current += step();
	return current;
}
