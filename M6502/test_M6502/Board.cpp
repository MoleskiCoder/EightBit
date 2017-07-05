#include "stdafx.h"
#include "Board.h"
#include "Disassembly.h"

#include "ProcessorType.h"

#include <iostream>
#include <sstream>
#include <iomanip>

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_memory(0xffff),
  m_cpu(EightBit::MOS6502(m_memory, EightBit::ProcessorType::Cpu6502)),
  m_symbols(""),
  m_disassembler(m_cpu, m_symbols),
  m_profiler(m_cpu, m_disassembler, m_symbols),
  m_oldPC(0xffff),
  m_stopped(false) {
}

void Board::initialise() {

	m_memory.clear();
	auto romDirectory = m_configuration.getRomDirectory();

	m_memory.loadRam(romDirectory + "/6502_functional_test.bin", 0);	// Klaus Dormann functional tests

	if (m_configuration.isProfileMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_StopLoop, this, std::placeholders::_1));
	m_cpu.ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_StopLoop, this, std::placeholders::_1));

	m_cpu.initialise();
	m_cpu.PC() = m_configuration.getStartAddress();
}

void Board::Cpu_ExecutingInstruction_Profile(const EightBit::MOS6502& cpu) {

	const auto pc = m_cpu.PC();

	//m_profiler.addAddress(pc.word, m_cpu.);
	//m_profiler.addInstruction(m_memory.peek(pc.word));
}

void Board::Cpu_ExecutingInstruction_StopLoop(const EightBit::MOS6502& cpu) {
	auto pc = m_cpu.PC().word;
	if (m_oldPC == pc) {
		m_cpu.halt();
		m_stopped = true;
	} else {
		m_oldPC = pc;
	}
}

void Board::Cpu_ExecutedInstruction_StopLoop(const EightBit::MOS6502& cpu) {
	if (m_stopped) {
		auto test = m_cpu.GetByte(0x0200);
		std::cout << std::endl << "** Test=" << std::hex << (int)test;
	}
}

void Board::Cpu_ExecutingInstruction_Debug(const EightBit::MOS6502& cpu) {

	auto address = m_cpu.PC().word;
	auto cell = m_memory.peek(address);

	const auto& instruction = m_cpu.getInstruction(cell);
	auto mode = instruction.mode;

	std::cout << std::hex;
	std::cout << "PC=" << std::setw(4) << std::setfill('0') << address << ":";
	std::cout << "P=" << (std::string)m_cpu.getP() << ", ";
	std::cout << std::setw(2);
	std::cout << "A=" << (int)m_cpu.getA() << ", ";
	std::cout << "X=" << (int)m_cpu.getX() << ", ";
	std::cout << "Y=" << (int)m_cpu.getY() << ", ";
	std::cout << "S=" << (int)m_cpu.getS() << "\t";

	std::cout << m_disassembler.Dump_ByteValue(cell);
	std::cout << m_disassembler.DumpBytes(mode, address + 1);

	std::cout << "\t ";

	std::cout << m_disassembler.Disassemble(address);

	std::cout << std::endl;
}
