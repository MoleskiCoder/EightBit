#include "stdafx.h"
#include "../inc/Profiler.h"

#include "../inc/Disassembly.h"

EightBit::Profiler::Profiler(mc6809& targetProcessor, Disassembly& disassemblerTarget)
: processor(targetProcessor),
  disassembler(disassemblerTarget) {
	instructionCounts.fill(0);
	addressProfiles.fill(0);
	addressCounts.fill(0);
}

void EightBit::Profiler::Generate() {
	StartingOutput.fire();
	EmitProfileInformation();
	StartingOutput.fire();
}

void EightBit::Profiler::EmitProfileInformation() {
	// For each memory address
	for (int address = 0; address < 0x10000; ++address) {
		// If there are any cycles associated
		auto cycles = addressProfiles[address];
		if (cycles > 0) {
			// Dump a profile/disassembly line
			auto source = disassembler.disassemble(address);
			ProfileLineEventArgs event(address, source, cycles);
			EmitLine.fire(event);
		}
	}
}

void EightBit::Profiler::addInstruction(uint8_t instruction) {
	++instructionCounts[instruction];
}

void EightBit::Profiler::addAddress(uint16_t address, int cycles) {
	addressCounts[address]++;
	addressProfiles[address] += cycles;
}
