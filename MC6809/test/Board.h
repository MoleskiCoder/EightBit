#pragma once

#include "Configuration.h"

#include <string>

#include <Ram.h>
#include <Rom.h>
#include <Bus.h>
#include <mc6809.h>
#include <Disassembly.h>
#include <MC6850.h>
#include <UnusedMemory.h>
#include <Profiler.h>

class Board : public EightBit::Bus {
public:
	Board(const Configuration& configuration);

	auto& CPU() { return m_cpu; }
	auto& ACIA() { return m_acia; }

	virtual void raisePOWER() final;
	virtual void lowerPOWER() final;

	virtual void initialise() final;

protected:
	virtual EightBit::MemoryMapping mapping(uint16_t address) final;

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x8000;							// 0000 - 7FFF, 32K RAM
	EightBit::UnusedMemory m_unused2000 = { 0x2000, 0xff };	// 8000 - 9FFF, 8K unused
	EightBit::Ram m_io = 0x2000;							// A000 - BFFF, 8K serial interface, minimally decoded
	EightBit::Rom m_rom = 0x4000;							// C000 - FFFF, 16K ROM

	EightBit::mc6850 m_acia;

	EightBit::mc6809 m_cpu = *this;
	EightBit::Disassembly m_disassembler = { *this, m_cpu };
	EightBit::Profiler m_profiler = { m_cpu, m_disassembler };

	uint64_t m_totalCycleCount = 0UL;
	int64_t m_frameCycleCount = 0L;

	// The m_disassembleAt and m_ignoreDisassembly are used to skip pin events
	EightBit::register16_t m_disassembleAt = 0x0000;
	bool m_ignoreDisassembly = false;

	void updateAciaPins();

	bool accessAcia();
};
