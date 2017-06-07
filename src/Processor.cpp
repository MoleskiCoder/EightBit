#include "stdafx.h"
#include "Processor.h"

EightBit::Processor::Processor(Memory& memory)
:	m_memory(memory),
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

void EightBit::Processor::push(uint8_t value) {
	sp.word--;
	m_memory.ADDRESS() = sp;
	m_memory.reference() = value;
}

void EightBit::Processor::pushWord(register16_t value) {
	push(value.high);
	push(value.low);
}

uint8_t EightBit::Processor::pop() {
	m_memory.ADDRESS() = sp;
	sp.word++;
	return m_memory.reference();
}

void EightBit::Processor::popWord(register16_t& output) {
	output.low = pop();
	output.high = pop();
}
