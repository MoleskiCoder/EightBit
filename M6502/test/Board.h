#pragma once

#include "Configuration.h"

#include <string>

#include <Ram.h>
#include <Bus.h>
#include <Disassembly.h>
#include <mos6502.h>
#include <Symbols.h>
#include <Register.h>

class Board : public EightBit::Bus {
public:
	Board(const Configuration& configuration);

	EightBit::MOS6502& CPU() { return m_cpu; }

	virtual void powerOn() final;
	virtual void powerOff() final;

protected:
	virtual void initialise() final;
	virtual EightBit::MemoryMapping mapping(uint16_t address) final {
		return { m_ram, 0x0000, 0xffff, EightBit::MemoryMapping::ReadWrite };
	}

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x10000;
	EightBit::MOS6502 m_cpu = *this;
	EightBit::Symbols m_symbols;
	EightBit::Disassembly m_disassembler = { *this, m_cpu, m_symbols };

	EightBit::register16_t m_oldPC = EightBit::Chip::Mask16;
	bool m_stopped = false;
};
