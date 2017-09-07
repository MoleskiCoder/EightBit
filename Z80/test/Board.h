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
	virtual uint8_t& reference(uint16_t address, bool& rom) {
		rom = false;
		return m_ram.reference(address);
	}

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram;
	EightBit::InputOutput m_ports;
	EightBit::Z80 m_cpu;
	EightBit::Disassembler m_disassembler;
	EightBit::Profiler m_profiler;

	void Cpu_ExecutingInstruction_Cpm(const EightBit::Z80& cpu);

	void Cpu_ExecutingInstruction_Debug(const EightBit::Z80& cpuEvent);
	void Cpu_ExecutingInstruction_Profile(const EightBit::Z80& cpuEvent);

	void bdos();
};
