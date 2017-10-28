#include "stdafx.h"
#include "Board.h"
#include "Disassembly.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <assert.h>

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_ram(0x10000),
  m_cpu(EightBit::MOS6502(*this)),
  m_symbols(""),
  m_disassembler(m_cpu, m_symbols),
  m_profiler(m_cpu, m_disassembler, m_symbols),
  m_oldPC(0xffff),
  m_stopped(false) {
}

void Board::initialise() {

	auto programFilename = m_configuration.getProgram();
	auto programPath = m_configuration.getRomDirectory() + "\\" + m_configuration.getProgram();
	auto loadAddress = m_configuration.getLoadAddress();

	switch (m_configuration.getLoadMethod()) {
	case Configuration::LoadMethod::Ram:
		m_ram.load(programPath, loadAddress);
		break;
	case Configuration::LoadMethod::Rom:
		// m_rom.load(programPath, loadAddress);
		break;
	}

	if (m_configuration.isProfileMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		m_cpu.ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	switch (m_configuration.getStopCondition()) {
	case Configuration::StopCondition::Loop:
		m_cpu.ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_StopLoop, this, std::placeholders::_1));
		break;
	case Configuration::StopCondition::Halt:
		break;
	default:
		throw std::domain_error("Unknown stop condition");
	}

	if (m_configuration.allowInput()) {
		ReadingByte.connect(std::bind(&Board::Memory_ReadingByte_Input, this, std::placeholders::_1));
		m_cpu.ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_Poll, this, std::placeholders::_1));
	}

	if (m_configuration.allowOutput())
		WrittenByte.connect(std::bind(&Board::Memory_WrittenByte_Output, this, std::placeholders::_1));

	m_pollCounter = 0;
	m_pollInterval = m_configuration.getPollInterval();

	m_cpu.initialise();
	m_cpu.PC().word = m_configuration.getStartAddress();
}

void Board::Cpu_ExecutingInstruction_Profile(const EightBit::MOS6502& cpu) {

	const auto pc = m_cpu.PC();

	//m_profiler.addAddress(pc.word, m_cpu.);
	//m_profiler.addInstruction(m_memory.peek(pc.word));
}

void Board::Cpu_ExecutedInstruction_StopLoop(const EightBit::MOS6502& cpu) {

	auto pc = m_cpu.PC().word;
	if (m_oldPC == pc) {
		m_cpu.halt();
		auto test = peek(0x0200);
		std::cout << std::endl << "** Test=" << std::hex << (int)test;
	} else {
		m_oldPC = pc;
	}
}

void Board::Cpu_ExecutingInstruction_Debug(const EightBit::MOS6502& cpu) {

	auto address = m_cpu.PC().word;
	auto cell = peek(address);

	std::cout << std::hex;
	std::cout << "PC=" << std::setw(4) << std::setfill('0') << address << ":";
	std::cout << "P=" << m_disassembler.dump_Flags(m_cpu.P()) << ", ";
	std::cout << std::setw(2) << std::setfill('0');
	std::cout << "A=" << (int)m_cpu.A() << ", ";
	std::cout << "X=" << (int)m_cpu.X() << ", ";
	std::cout << "Y=" << (int)m_cpu.Y() << ", ";
	std::cout << "S=" << (int)m_cpu.S() << "\t";

	std::cout << m_disassembler.disassemble(address);

	std::cout << "\n";
}

void Board::Memory_ReadingByte_Input(const uint16_t address) {
	if (address == m_configuration.getInputAddress()) {
		if (DATA() != 0) {
			assert(address == ADDRESS().word);
			write(0);
		}
	}
}

void Board::Memory_WrittenByte_Output(const uint16_t address) {
	if (address == m_configuration.getOutputAddress()) {
		_putch(DATA());
	}
}

void Board::Cpu_ExecutedInstruction_Poll(const EightBit::MOS6502& cpu) {
	if (++m_pollCounter == m_pollInterval) {
		m_pollCounter = 0;
		pollKeyboard();
	}
}

void Board::pollKeyboard() {
	if (_kbhit())
		poke(m_configuration.getInputAddress(), _getch());
}