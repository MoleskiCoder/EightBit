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
	virtual uint8_t& reference(uint16_t address) {
		return m_ram.reference(address);
	}

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x10000;
	EightBit::MOS6502 m_cpu;
	EightBit::Symbols m_symbols;
	EightBit::Disassembly m_disassembler;
	EightBit::Profiler m_profiler;

	uint16_t m_oldPC = 0xffff;
	bool m_stopped = false;
	uint64_t m_pollCounter = 0UL;
	uint64_t m_pollInterval = 0UL;

	void pollKeyboard();

	void Cpu_ExecutingInstruction_Debug(const EightBit::MOS6502& cpu);
	void Cpu_ExecutingInstruction_Profile(const EightBit::MOS6502& cpu);

	void Cpu_ExecutedInstruction_StopLoop(const EightBit::MOS6502& cpu);

	void Memory_ReadingByte_Input(EightBit::EventArgs);
	void Memory_WrittenByte_Output(EightBit::EventArgs);

	void Cpu_ExecutedInstruction_Poll(const EightBit::MOS6502& cpu);
};
