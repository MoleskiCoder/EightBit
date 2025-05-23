#pragma once

#include <array>
#include <cstdint>
#include <map>

#include <EventArgs.h>
#include <Signal.h>

#include "ProfileLineEventArgs.h"
#include "ProfileScopeEventArgs.h"

namespace EightBit {

	class Disassembly;
	class MOS6502;
	class Symbols;

	class Profiler {
	public:
		std::array<uint64_t, 0x100> instructionCounts;
		std::array<uint64_t, 0x10000> addressProfiles;
		std::array<uint64_t, 0x10000> addressCounts;

		std::array<std::string, 0x10000> addressScopes;
		std::map<std::string, uint64_t> scopeCycles;

		MOS6502& processor;
		Disassembly& disassembler;
		const Symbols& symbols;

		Profiler(MOS6502& processor, Disassembly& disassembler, Symbols& symbols);

		Signal<EventArgs> StartingOutput;
		Signal<EventArgs> FinishedOutput;

		Signal<EventArgs> StartingLineOutput;
		Signal<EventArgs> FinishedLineOutput;

		Signal<ProfileLineEventArgs> EmitLine;

		Signal<EventArgs> StartingScopeOutput;
		Signal<EventArgs> FinishedScopeOutput;

		Signal<ProfileScopeEventArgs> EmitScope;

		void addInstruction(uint8_t instruction);
		void addAddress(uint16_t address, int cycles);

		void Generate();

	private:
		void EmitProfileInformation();

		void BuildAddressScopes();
	};
}