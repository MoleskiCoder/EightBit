#include "stdafx.h"
#include "Profiler.h"

Profiler::Profiler(System6502& targetProcessor, Disassembly& disassemblerTarget, Symbols& symbolsTarget, bool instructions, bool addresses)
:	processor(targetProcessor),
	disassembler(disassemblerTarget),
	symbols(symbolsTarget),
	countInstructions(instructions),
	profileAddresses(addresses)
{
	instructionCounts.fill(0);
	addressProfiles.fill(0);
	addressCounts.fill(0);

	if (profileAddresses)
		processor.ExecutingInstruction.connect(std::bind(&Profiler::Processor_ExecutingInstruction_ProfileAddresses, this, std::placeholders::_1));
	if (countInstructions)
		processor.ExecutingInstruction.connect(std::bind(&Profiler::Processor_ExecutingInstruction_CountInstructions, this, std::placeholders::_1));
	if (profileAddresses)
		processor.ExecutedInstruction.connect(std::bind(&Profiler::Processor_ExecutedInstruction_ProfileAddresses, this, std::placeholders::_1));

	BuildAddressScopes();
}

void Profiler::Generate() {
	StartingOutput.fire(EventArgs());
	EmitProfileInformation();
	StartingOutput.fire(EventArgs());
}

void Profiler::EmitProfileInformation() {

	{
		StartingLineOutput.fire(EventArgs());
		// For each memory address
		for (uint16_t address = 0; address < 0x10000; ++address) {
			// If there are any cycles associated
			auto cycles = addressProfiles[address];
			if (cycles > 0) {
				// Dump a profile/disassembly line
				auto source = disassembler.Disassemble(address);
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
			auto namedAddress = (size_t)symbols.getAddresses().find(name)->second;
			auto count = addressCounts[namedAddress];
			EmitScope.fire(ProfileScopeEventArgs(name, cycles, count));
		}
		FinishedScopeOutput.fire(EventArgs());
	}
}

void Profiler::Processor_ExecutingInstruction_ProfileAddresses(const AddressEventArgs& addressEvent) {
	assert(profileAddresses);
	priorCycleCount = processor.getCycles();
	addressCounts[addressEvent.getAddress()]++;
}

void Profiler::Processor_ExecutingInstruction_CountInstructions(const AddressEventArgs& addressEvent) {
	assert(countInstructions);
	++instructionCounts[addressEvent.getCell()];
}

void Profiler::Processor_ExecutedInstruction_ProfileAddresses(const AddressEventArgs& addressEvent) {
	assert(profileAddresses);
	auto cycles = processor.getCycles() - priorCycleCount;
	addressProfiles[addressEvent.getAddress()] += cycles;
	auto addressScope = addressScopes[addressEvent.getAddress()];
	if (!addressScope.empty()) {
		if (scopeCycles.find(addressScope) == scopeCycles.end())
			scopeCycles[addressScope] = 0;
		scopeCycles[addressScope] += cycles;
	}
}

void Profiler::BuildAddressScopes() {
	for (auto& label : symbols.getLabels()) {
		auto address = label.first;
		auto key = label.second;
		auto scope = symbols.getScopes().find(key);
		if (scope != symbols.getScopes().end()) {
			for (uint16_t i = address; i < address + scope->second; ++i) {
				addressScopes[i] = key;
			}
		}
	}
}