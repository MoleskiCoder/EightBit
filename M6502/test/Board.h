#pragma once

#include "Configuration.h"

#include <string>

#include <Ram.h>
#include <Bus.h>
#include <Profiler.h>
#include <Disassembly.h>
#include <mos6502.h>
#include <Symbols.h>

class Board : public EightBit::Bus {
public:
	Board(const Configuration& configuration);

	EightBit::MOS6502& CPU() { return m_cpu; }

	void initialise();

protected:
	virtual uint8_t& reference(uint16_t address, bool& rom) {
		rom = false;
		return m_ram.reference(address);
	}

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram;
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

	void Memory_ReadingByte_Input(uint16_t address);
	void Memory_WrittenByte_Output(uint16_t address);

	void Cpu_ExecutedInstruction_Poll(const EightBit::MOS6502& cpu);
};