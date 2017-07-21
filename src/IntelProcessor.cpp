#include "stdafx.h"
#include "IntelProcessor.h"

EightBit::IntelProcessor::IntelProcessor(Memory& memory)
: Processor(memory) {
	MEMPTR().word = 0;
	SP().word = 0xffff;
}

void EightBit::IntelProcessor::initialise() {
	Processor::initialise();

	for (int i = 0; i < 0x100; ++i) {
		m_decodedOpcodes[i] = i;
	}

	MEMPTR().word = 0;
	SP().word = 0xffff;
}
