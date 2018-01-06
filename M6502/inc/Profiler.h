#pragma once

#include <array>
#include <map>
#include <cstdint>
#include <functional>

#include <EventArgs.h>
#include <Signal.h>

#include "Disassembly.h"
#include "Symbols.h"

#include "ProfileLineEventArgs.h"
#include "ProfileScopeEventArgs.h"

#include "mos6502.h"

namespace EightBit {
	class Profiler {
	public:
		std::array<uint64_t, 0x100> instructionCounts;
		std::array<uint64_t, 0x10000> addressProfiles;
		std::array<uint64_t, 0x10000> addressCounts;

		std::array<std::string, 0x10000> addressScopes;
		std::map<std::string, uint64_t> scopeCycles;

		MOS6502& processor;
		const Disassembly& disassembler;
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