#include "stdafx.h"
#include "../inc/Profiler.h"

#include "../inc/Disassembly.h"
#include "../inc/Symbols.h"

EightBit::Profiler::Profiler(MOS6502& targetProcessor, Disassembly& disassemblerTarget, Symbols& symbolsTarget)
: processor(targetProcessor),
  disassembler(disassemblerTarget),
  symbols(symbolsTarget) {

	instructionCounts.fill(0);
	addressProfiles.fill(0);
	addressCounts.fill(0);

	BuildAddressScopes();
}

void EightBit::Profiler::Generate() {
	StartingOutput.fire();
	EmitProfileInformation();
	StartingOutput.fire();
}

void EightBit::Profiler::EmitProfileInformation() {

	{
		StartingLineOutput.fire();
		// For each memory address
		for (int address = 0; address < 0x10000; ++address) {
			// If there are any cycles associated
			auto cycles = addressProfiles[address];
			if (cycles > 0) {
				// Dump a profile/disassembly line
				auto source = disassembler.disassemble(address);
				ProfileLineEventArgs event(source, cycles);
				EmitLine.fire(event);
			}
		}
		FinishedLineOutput.fire();
	}

	{
		StartingScopeOutput.fire();
		for (const auto& scopeCycle : scopeCycles) {
			const auto& [name, cycles] = scopeCycle;
			auto namedAddress = (size_t)symbols.addresses().find(name)->second;
			auto count = addressCounts[namedAddress];
			ProfileScopeEventArgs event(name, cycles, count);
			EmitScope.fire(event);
		}
		FinishedScopeOutput.fire();
	}
}

void EightBit::Profiler::addInstruction(uint8_t instruction) {
	++instructionCounts[instruction];
}

void EightBit::Profiler::addAddress(uint16_t address, int cycles) {

	addressCounts[address]++;

	addressProfiles[address] += cycles;
	auto addressScope = addressScopes[address];
	if (!addressScope.empty()) {
		if (scopeCycles.find(addressScope) == scopeCycles.end())
			scopeCycles[addressScope] = 0;
		scopeCycles[addressScope] += cycles;
	}
}

void EightBit::Profiler::BuildAddressScopes() {
	for (const auto& label : symbols.labels()) {
		const auto& [address, key] = label;
		auto scope = symbols.scopes().find(key);
		if (scope != symbols.scopes().end()) {
			for (uint16_t i = address; i < address + scope->second; ++i) {
				addressScopes[i] = key;
			}
		}
	}
}
