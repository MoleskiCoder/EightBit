#include "stdafx.h"
#include "IntelProcessor.h"

EightBit::IntelProcessor::IntelProcessor(Bus& bus)
: Processor(bus) {
	SP().word = Mask16;
}

void EightBit::IntelProcessor::initialise() {
	Processor::initialise();
	for (int i = 0; i < 0x100; ++i) {
		m_decodedOpcodes[i] = i;
	}
}

void EightBit::IntelProcessor::reset() {
	Processor::reset();
	SP().word = AF().word = BC().word = DE().word = HL().word = Mask16;
}