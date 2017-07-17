#include "stdafx.h"
#include "IntelProcessor.h"

EightBit::IntelProcessor::IntelProcessor(Memory& memory)
: Processor(memory) {
	MEMPTR().word = 0;
	SP().word = 0xffff;
}

void EightBit::IntelProcessor::initialise() {
	Processor::initialise();
	MEMPTR().word = 0;
	SP().word = 0xffff;
}
