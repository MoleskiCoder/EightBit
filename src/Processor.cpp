#include "stdafx.h"
#include "Processor.h"

EightBit::Processor::Processor(Memory& memory)
:	m_memory(memory),
	cycles(0),
	m_halted(false) {
	pc.word = sp.word = 0;
}

void EightBit::Processor::reset() {
	pc.word = 0;
}

void EightBit::Processor::initialise() {
	sp.word = 0;
	reset();
}

void EightBit::Processor::pushWord(register16_t value) {
	sp.word -= 2;
	setWord(sp.word, value);
}

EightBit::register16_t EightBit::Processor::popWord() {
	auto value = getWord(sp.word);
	sp.word += 2;
	return value;
}
