#include "stdafx.h"
#include "Profiler.h"

#include "Disassembly.h"
#include "Symbols.h"

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
	StartingOutput.fire(EventArgs());
	EmitProfileInformation();
	StartingOutput.fire(EventArgs());
}

void EightBit::Profiler::EmitProfileInformation() {

	{
		StartingLineOutput.fire(EventArgs());
		// For each memory address
		for (int address = 0; address < 0x10000; ++address) {
			// If there are any cycles associated
			auto cycles = addressProfiles[address];
			if (cycles > 0) {
				// Dump a profile/disassembly line
				auto source = disassembler.disassemble(address);
				EmitLine.fire(ProfileLineEventArgs(source, cycles));
			}
		}
		FinishedLineOutput.fire(EventArgs());
	}

	{
		StartingScopeOutput.fire(EventArgs());
		for (auto& scopeCycle : scopeCycles) {
			auto name = scopeCycle.first;
			auto cycles = scopeCycle.second;
			auto namedAddress = (size_t)symbols.addresses().find(name)->second;
			auto count = addressCounts[namedAddress];
			EmitScope.fire(ProfileScopeEventArgs(name, cycles, count));
		}
		FinishedScopeOutput.fire(EventArgs());
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
	for (auto& label : symbols.labels()) {
		auto address = label.first;
		auto key = label.second;
		auto scope = symbols.scopes().find(key);
		if (scope != symbols.scopes().end()) {
			for (uint16_t i = address; i < address + scope->second; ++i) {
				addressScopes[i] = key;
			}
		}
	}
}