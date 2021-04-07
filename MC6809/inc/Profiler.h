#pragma once

#include <array>
#include <cstdint>
#include <map>

#include <EventArgs.h>
#include <Signal.h>

#include "ProfileLineEventArgs.h"

namespace EightBit {

	class Disassembly;
	class mc6809;

	class Profiler {
	public:
		std::array<uint64_t, 0x100> instructionCounts;
		std::array<uint64_t, 0x10000> addressProfiles;
		std::array<uint64_t, 0x10000> addressCounts;

		mc6809& processor;
		Disassembly& disassembler;

		Profiler(mc6809& processor, Disassembly& disassembler);

		Signal<EventArgs> StartingOutput;
		Signal<EventArgs> FinishedOutput;

		Signal<ProfileLineEventArgs> EmitLine;

		void addInstruction(uint8_t instruction);
		void addAddress(uint16_t address, int cycles);

		void Generate();

	private:
		void EmitProfileInformation();
	};
}