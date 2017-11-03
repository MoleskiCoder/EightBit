#include "stdafx.h"
#include "Processor.h"

EightBit::Processor::Processor(Bus& bus)
: m_bus(bus),
  m_cycles(0),
  m_halted(false),
  m_power(false) {
	PC().word = MEMPTR().word = 0;
}

void EightBit::Processor::reset() {
	PC().word = MEMPTR().word = 0;
}

void EightBit::Processor::initialise() {
	reset();
}

int EightBit::Processor::run(int limit) {
	int current = 0;
	while (powered() && current < limit) {
		current += singleStep();
	}
	return current;
}

int EightBit::Processor::singleStep() {
	return step();
}
