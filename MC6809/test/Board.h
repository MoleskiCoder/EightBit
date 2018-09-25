#pragma once

#include "Configuration.h"

#include <string>

#include <Ram.h>
#include <Bus.h>
#include <mc6809.h>
#include <Disassembly.h>
#include <MC6850.h>

class Board : public EightBit::Bus {
public:
	Board(const Configuration& configuration);

	EightBit::mc6809& CPU() { return m_cpu; }
	EightBit::mc6850& ACIA() { return m_acia; }

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

	EightBit::mc6850 m_acia;

	EightBit::mc6809 m_cpu;
	EightBit::Disassembly m_disassembler;

	EightBit::register16_t m_disassembleAt = 0x0000;
	bool m_ignoreDisassembly = false;

	void Cpu_ExecutingInstruction_Debug(EightBit::mc6809&);
	void Cpu_ExecutedInstruction_Debug(EightBit::mc6809&);

	void Cpu_ExecutedInstruction_die(EightBit::mc6809&);

	// ACIA handling

	void Bus_WritingByte_Acia(EightBit::EventArgs&);
	void Bus_ReadingByte_Acia(EightBit::EventArgs&);

	void Cpu_ExecutedInstruction_Acia(EightBit::mc6809&);

	void Acia_Accessing(EightBit::EventArgs&);
	void Acia_Accessed(EightBit::EventArgs&);

	void Acia_Transmitting(EightBit::EventArgs&);
	void Acia_Transmitted(EightBit::EventArgs&);

	void Acia_Receiving(EightBit::EventArgs&);
	void Acia_Received(EightBit::EventArgs&);

	void updateAciaPins(EightBit::Chip::PinLevel rw);
};
