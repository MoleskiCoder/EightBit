#include "stdafx.h"
#include "Processor.h"

EightBit::Processor::Processor(Memory& memory)
: m_memory(memory),
  cycles(0),
  m_halted(false) {
	PC().word = 0;
}

void EightBit::Processor::reset() {
	PC().word = 0;
}

void EightBit::Processor::initialise() {
	reset();
}
