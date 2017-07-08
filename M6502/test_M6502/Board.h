#pragma once

#include <string>

#include "Memory.h"
#include "Configuration.h"
#include "Profiler.h"
#include "EventArgs.h"
#include "Disassembly.h"
#include "mos6502.h"
#include "Symbols.h"

class Board {
public:
	Board(const Configuration& configuration);

	EightBit::Memory& Memory() { return m_memory; }
	EightBit::MOS6502& CPU() { return m_cpu; }

	void initialise();

private:
	const Configuration& m_configuration;
	EightBit::Memory m_memory;
	EightBit::MOS6502 m_cpu;
	EightBit::Symbols m_symbols;
	EightBit::Disassembly m_disassembler;
	EightBit::Profiler m_profiler;

	uint16_t m_oldPC;
	bool m_stopped;
	uint64_t m_pollCounter;
	uint64_t m_pollInterval;

	void pollKeyboard();

	void Cpu_ExecutingInstruction_Debug(const EightBit::MOS6502& cpu);
	void Cpu_ExecutingInstruction_Profile(const EightBit::MOS6502& cpu);

	void Cpu_ExecutedInstruction_StopLoop(const EightBit::MOS6502& cpu);

	void Memory_ReadByte_Input(const EightBit::AddressEventArgs& e);
	void Memory_WrittenByte_Output(const EightBit::AddressEventArgs& e);

	void Cpu_ExecutedInstruction_Poll(const EightBit::MOS6502& cpu);
};
