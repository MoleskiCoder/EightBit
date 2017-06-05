#pragma once

//#include <string>

#include "Memory.h"
#include "InputOutput.h"
#include "Intel8080.h"
#include "Profiler.h"
#include "EventArgs.h"

class Configuration;

class Board {
public:
	Board(const Configuration& configuration);

	EightBit::Memory& getMemory() { return m_memory; }
	const EightBit::Intel8080& getCPU() const { return m_cpu; }
	EightBit::Intel8080& getCPUMutable() { return m_cpu; }

	void initialise();

private:
	const Configuration& m_configuration;
	EightBit::Memory m_memory;
	EightBit::InputOutput m_ports;
	EightBit::Intel8080 m_cpu;
	EightBit::Profiler m_profiler;

	void Cpu_ExecutingInstruction_Cpm(const EightBit::Intel8080& cpu);

	void Cpu_ExecutingInstruction_Debug(const EightBit::Intel8080& cpuEvent);
	void Cpu_ExecutingInstruction_Profile(const EightBit::Intel8080& cpuEvent);

	void bdos();
};
