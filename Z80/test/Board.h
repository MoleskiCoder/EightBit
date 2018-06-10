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
	const EightBit::Z80& CPU() const { return m_cpu; }

	void initialise();

protected:
	virtual uint8_t& reference(uint16_t address) {
		return m_ram.reference(address);
	}

	virtual uint8_t reference(uint16_t address) const {
		return m_ram.reference(address);
	}

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x10000;
	EightBit::InputOutput m_ports;
	EightBit::Z80 m_cpu;
	EightBit::Disassembler m_disassembler;
	EightBit::Profiler m_profiler;

	void Cpu_ExecutingInstruction_Cpm(const EightBit::Z80& cpu);

	void Cpu_ExecutingInstruction_Debug(const EightBit::Z80& cpu);
	void Cpu_ExecutingInstruction_Profile(const EightBit::Z80& cpu);

	void bdos() const;
};
