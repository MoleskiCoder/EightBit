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

	EightBit::Z80& CPU() noexcept { return m_cpu; }

	void raisePOWER() noexcept final;
	void lowerPOWER() noexcept final;

	void initialise() final;

protected:
	EightBit::MemoryMapping mapping(uint16_t address) noexcept final {
		return m_mapping;
	}

private:
	const Configuration& m_configuration;
	EightBit::Ram m_ram = 0x10000;
	EightBit::InputOutput m_ports;
	EightBit::Z80 m_cpu{ *this, m_ports };
	EightBit::Disassembler m_disassembler = *this;
	EightBit::Profiler m_profiler = { m_cpu, m_disassembler };
	const EightBit::MemoryMapping m_mapping = { m_ram, 0x0000, 0xffff, EightBit::MemoryMapping::AccessLevel::ReadWrite };
	int m_warmstartCount = 0;

	void bdos();
};
