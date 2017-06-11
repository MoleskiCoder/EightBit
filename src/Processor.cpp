#include "stdafx.h"
#include "Processor.h"

EightBit::Processor::Processor(Memory& memory)
: m_memory(memory),
  cycles(0),
  m_halted(false) {
	sp.word = 0xffff;
	pc.word = 0;
}

void EightBit::Processor::reset() {
	pc.word = 0;
}

void EightBit::Processor::initialise() {
	sp.word = 0xffff;
	reset();
}
