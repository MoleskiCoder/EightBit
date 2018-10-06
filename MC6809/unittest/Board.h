#pragma once

#include <Ram.h>
#include <Bus.h>
#include <mc6809.h>

class Board : public EightBit::Bus {
public:
	Board();

	EightBit::mc6809& CPU() { return m_cpu; }

	void initialise();

protected:
	virtual EightBit::MemoryMapping mapping(uint16_t address) final;

private:
	EightBit::Ram m_ram = 0x10000;	// 0000 - FFFF, 64K RAM
	EightBit::mc6809 m_cpu;
};
