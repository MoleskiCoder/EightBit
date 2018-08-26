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
	virtual uint8_t& reference(uint16_t address) final;

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x8000;			// 0000 - 7FFF, 32K RAM
	EightBit::Rom m_extendedBasic = 0x2000;	// 8000 - 9FFF, 8K Extended BASIC ROM
	EightBit::Rom m_colorBasic = 0x2000;	// A000 - BFFF, 8K Color BASIC ROM
	EightBit::Rom m_cartridge = 0x2000;		// C000 - DFFF, 8K Cartridge ROM
	EightBit::Rom m_diskBasic = 0x2000;		// C000 - DFFF, 8K Disk BASIC ROM
											// E000 - FEFF, Unused
	EightBit::Ram m_io = 0xf0;				// FF00 - FFEF, I/O Registers
											// FFF0 - FFFF, Interrupt vectors (mapped from BFF0-BFFF)
	EightBit::mc6809 m_cpu;
	EightBit::Disassembly m_disassembler;

	void pollKeyboard();

	void Cpu_ExecutingInstruction_Debug(EightBit::mc6809& cpu);
};
