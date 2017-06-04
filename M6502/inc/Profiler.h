#pragma once

#include <array>
#include <map>
#include <cstdint>
#include <functional>

#include <system6502.h>
#include <EventArgs.h>
#include <Signal.h>

#include "Disassembly.h"
#include "Symbols.h"

#include "ProfileLineEventArgs.h"
#include "ProfileScopeEventArgs.h"

class Profiler
{
public:
	std::array<uint64_t, 0x100> instructionCounts;
	std::array<uint64_t, 0x10000> addressProfiles;
	std::array<uint64_t, 0x10000> addressCounts;

	std::array<std::string, 0x10000> addressScopes;
	std::map<std::string, uint64_t> scopeCycles;

	System6502& processor;
	const Disassembly& disassembler;
	const Symbols& symbols;

	bool countInstructions;
	bool profileAddresses;

	uint64_t priorCycleCount = 0;

	Profiler(System6502& processor, Disassembly& disassembler, Symbols& symbols, bool countInstructions, bool profileAddresses);

	Signal<EventArgs> StartingOutput;
	Signal<EventArgs> FinishedOutput;

	Signal<EventArgs> StartingLineOutput;
	Signal<EventArgs> FinishedLineOutput;

	Signal<ProfileLineEventArgs> EmitLine;

	Signal<EventArgs> StartingScopeOutput;
	Signal<EventArgs> FinishedScopeOutput;

	Signal<ProfileScopeEventArgs> EmitScope;

	void Generate();

private:
	void EmitProfileInformation();

	void Processor_ExecutingInstruction_ProfileAddresses(const AddressEventArgs& addressEvent);
	void Processor_ExecutingInstruction_CountInstructions(const AddressEventArgs& addressEvent);
	void Processor_ExecutedInstruction_ProfileAddresses(const AddressEventArgs& addressEvent);

	void BuildAddressScopes();
};
