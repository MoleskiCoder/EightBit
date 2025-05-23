#pragma once

#include <Ram.h>
#include <Bus.h>
#include <mc6809.h>
#include <Disassembly.h>

class Board final : public EightBit::Bus {
public:
	Board();

	constexpr EightBit::mc6809& CPU() noexcept { return m_cpu; }

	void raisePOWER() noexcept final;
	void lowerPOWER() noexcept final;

protected:
	void initialise() final;

	EightBit::MemoryMapping mapping(uint16_t address) noexcept final;

private:
	EightBit::Ram m_ram = 0x10000;	// 0000 - FFFF, 64K RAM
	EightBit::mc6809 m_cpu = { *this };
	EightBit::Disassembly m_disassembler = { *this, m_cpu };

	// The m_disassembleAt and m_ignoreDisassembly are used to skip pin events
	EightBit::register16_t m_disassembleAt = 0x0000;
	bool m_ignoreDisassembly = false;
};
