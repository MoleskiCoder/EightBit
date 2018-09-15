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

	void initialise();

protected:
	virtual EightBit::MemoryMapping mapping(uint16_t address) final {
		return { m_ram, 0x0000, EightBit::MemoryMapping::ReadWrite };
	}

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x10000;
	EightBit::InputOutput m_ports;
	EightBit::Z80 m_cpu;
	EightBit::Disassembler m_disassembler;
	EightBit::Profiler m_profiler;
	int m_warmstartCount = 0;

	void Cpu_ExecutingInstruction_Cpm(EightBit::Z80& cpu);

	void Cpu_ExecutingInstruction_Debug(EightBit::Z80& cpu);
	void Cpu_ExecutingInstruction_Profile(EightBit::Z80& cpu);

	void bdos();
};
