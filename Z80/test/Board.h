#pragma once

#include <string>

#include <Ram.h>
#include <Bus.h>
#include <InputOutput.h>
#include <Profiler.h>
#include <EventArgs.h>
#include <Disassembler.h>
#include <Z80.h>

#include "Configuration.h"

class Board : public EightBit::Bus {
public:
	Board(const Configuration& configuration);

	EightBit::Z80& CPU() { return m_cpu; }

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
	EightBit::InputOutput m_ports;
	EightBit::Z80 m_cpu = { *this, m_ports };
	EightBit::Disassembler m_disassembler = *this;
	EightBit::Profiler m_profiler = { m_cpu, m_disassembler };
	int m_warmstartCount = 0;

	void bdos();
};
