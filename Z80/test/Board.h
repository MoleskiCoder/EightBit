#pragma once

#include <string>

#include "Memory.h"
#include "InputOutput.h"
#include "Configuration.h"
#include "Profiler.h"
#include "EventArgs.h"
#include "Disassembler.h"
#include "Z80.h"

class Board {
public:
	Board(const Configuration& configuration);

	EightBit::Memory& Memory() { return m_memory; }
	EightBit::Z80& CPU() { return m_cpu; }

	void initialise();

private:
	const Configuration& m_configuration;
	EightBit::Memory m_memory;
	EightBit::InputOutput m_ports;
	EightBit::Z80 m_cpu;
	EightBit::Disassembler m_disassembler;
	EightBit::Profiler m_profiler;

	void Cpu_ExecutingInstruction_Cpm(const EightBit::Z80& cpu);

	void Cpu_ExecutingInstruction_Debug(const EightBit::Z80& cpuEvent);
	void Cpu_ExecutingInstruction_Profile(const EightBit::Z80& cpuEvent);

	void bdos();
};
