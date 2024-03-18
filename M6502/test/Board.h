#pragma once

#include "Configuration.h"

#include <string>

#include <Bus.h>
#include <Disassembly.h>
#include <mos6502.h>
#include <Ram.h>
#include <Register.h>
#include <Symbols.h>

class Board : public EightBit::Bus {
public:
	Board(const Configuration& configuration);

	EightBit::MOS6502& CPU() { return m_cpu; }

	void raisePOWER() noexcept final;
	void lowerPOWER() noexcept final;

	void initialise() final;

protected:
	virtual constexpr EightBit::MemoryMapping mapping(uint16_t address) noexcept final {
		return m_mapping;
	}

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x10000;
	EightBit::MOS6502 m_cpu = *this;
	EightBit::Symbols m_symbols;
	EightBit::Disassembly m_disassembler = { *this, m_cpu, m_symbols };
	const EightBit::MemoryMapping m_mapping = { m_ram, 0x0000, 0xffff, EightBit::MemoryMapping::AccessLevel::ReadWrite };

	EightBit::register16_t m_oldPC = EightBit::Chip::Mask16;
	bool m_stopped = false;
};
