#include "stdafx.h"
#include "IntelProcessor.h"

EightBit::IntelProcessor::IntelProcessor(Memory& memory)
: Processor(memory) {
	MEMPTR().word = 0;
}

EightBit::IntelProcessor::~IntelProcessor() {
}

void EightBit::IntelProcessor::initialise() {
	Processor::initialise();
	MEMPTR().word = 0;
}


void EightBit::IntelProcessor::push(uint8_t value) {
	m_memory.ADDRESS().word = --sp.word;
	m_memory.reference() = value;
}

void EightBit::IntelProcessor::pushWord(register16_t value) {
	push(value.high);
	push(value.low);
}

uint8_t EightBit::IntelProcessor::pop() {
	m_memory.ADDRESS().word = sp.word++;
	return m_memory.reference();
}

void EightBit::IntelProcessor::popWord(register16_t& output) {
	output.low = pop();
	output.high = pop();
}
