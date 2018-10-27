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

	auto& CPU() { return m_cpu; }
	auto& ACIA() { return m_acia; }

	void initialise();

protected:
	virtual EightBit::MemoryMapping mapping(uint16_t address) final;

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x8000;			// 0000 - 7FFF, 32K RAM
	EightBit::Rom m_unused2000 = 0x2000;	// 8000 - 9FFF, 8K unused
	EightBit::Ram m_io = 0x2000;			// A000 - BFFF, 8K serial interface, minimally decoded
	EightBit::Rom m_rom = 0x4000;			// C000 - FFFF, 16K ROM

	EightBit::mc6850 m_acia;

	EightBit::mc6809 m_cpu;
	EightBit::Disassembly m_disassembler;

	uint64_t m_totalCycleCount = 0UL;
	int64_t m_frameCycleCount = 0L;

	// The m_disassembleAt and m_ignoreDisassembly are used to skip pin events
	EightBit::register16_t m_disassembleAt = 0x0000;
	bool m_ignoreDisassembly = false;

	// CPU events

	void Cpu_ExecutingInstruction_Debug(EightBit::mc6809&);
	void Cpu_ExecutedInstruction_Debug(EightBit::mc6809&);

	void Cpu_ExecutedInstruction_Terminator(EightBit::mc6809&);
	void Cpu_ExecutedInstruction_Acia(EightBit::mc6809&);

	// Bus events

	// Allows us to marshal data from memory -> ACIA
	void Bus_WrittenByte_Acia(EightBit::EventArgs&);

	// Allows us to marshal data from ACIA -> memory
	void Bus_ReadingByte_Acia(EightBit::EventArgs&);

	// ACIA events

	// Allows us to catch a byte being transmitted
	void Acia_Transmitting(EightBit::EventArgs&);

	// Use the bus data to update the ACIA access/address pins
	void updateAciaPins(EightBit::Chip::PinLevel rw);

	bool accessAcia();
};
