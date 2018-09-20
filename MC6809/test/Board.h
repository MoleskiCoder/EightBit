#pragma once

#include "Configuration.h"

#include <string>

#include <Ram.h>
#include <Bus.h>
#include <mc6809.h>
#include <Disassembly.h>

class Board : public EightBit::Bus {
public:
	Board(const Configuration& configuration);

	EightBit::mc6809& CPU() { return m_cpu; }

	void initialise();

protected:
	virtual EightBit::MemoryMapping mapping(uint16_t address) final;

private:

	enum {
		Uart = 0xa000,
		Ustat = Uart,
		Uctrl = Uart,
		Recev = Uart + 1,
		Trans = Uart + 1,
	};

	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x8000;			// 0000 - 7FFF, 32K RAM
	EightBit::Rom m_unused2000 = 0x2000;	// 8000 - 9FFF, 8K unused
	EightBit::Ram m_io = 0x2000;			// A000 - BFFF, 8K serial interface, minimally decoded
	EightBit::Rom m_rom = 0x4000;			// C000 - FFFF, 16K ROM

	EightBit::mc6809 m_cpu;
	EightBit::Disassembly m_disassembler;

	EightBit::register16_t m_disassembleAt = 0x0000;
	bool m_ignoreDisassembly = false;

	void pollKeyboard();

	void Cpu_ExecutingInstruction_Debug(EightBit::mc6809& cpu);
	void Cpu_ExecutedInstruction_Debug(EightBit::mc6809& cpu);

	void Cpu_ExecutedInstruction_die(EightBit::mc6809& cpu);

	void Bus_WrittenByte(EightBit::EventArgs&);
	void Bus_ReadingByte(EightBit::EventArgs&);
};
