#pragma once

#include <Bus.h>
#include <Ram.h>
#include <InputOutput.h>
#include <Intel8080.h>
#include <Profiler.h>
#include <EventArgs.h>
#include <Disassembler.h>

class Configuration;

class Board : public EightBit::Bus {
public:
	Board(const Configuration& configuration);

	EightBit::Intel8080& CPU() { return m_cpu; }
	const EightBit::Intel8080& CPU() const { return m_cpu; }

	virtual void raisePOWER() final;
	virtual void lowerPOWER() final;

	virtual void initialise() final;

protected:
	virtual EightBit::MemoryMapping mapping(uint16_t address) noexcept final {
		return { m_ram, 0x0000, 0xffff, EightBit::MemoryMapping::AccessLevel::ReadWrite };
	}

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x10000;
	EightBit::InputOutput m_ports;
	EightBit::Intel8080 m_cpu;
	EightBit::Disassembler m_disassembler;
	EightBit::Profiler m_profiler;
	int m_warmstartCount = 0;

	void Cpu_ExecutingInstruction_Cpm(EightBit::Intel8080& cpu);

	void Cpu_ExecutingInstruction_Debug(const EightBit::Intel8080& cpu);
	void Cpu_ExecutingInstruction_Profile(EightBit::Intel8080& cpu);

	void bdos();
};
