#include "stdafx.h"
#include "Board.h"
#include "Disassembly.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <assert.h>

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::MOS6502(*this)),
  m_disassembler(m_cpu, m_symbols),
  m_profiler(m_cpu, m_disassembler, m_symbols) {}

void Board::initialise() {

	auto programFilename = m_configuration.getProgram();
	auto programPath = m_configuration.getRomDirectory() + "/" + m_configuration.getProgram();
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
		CPU().ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Profile, this, std::placeholders::_1));
	}

	if (m_configuration.isDebugMode()) {
		CPU().ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
	}

	switch (m_configuration.getStopCondition()) {
	case Configuration::StopCondition::Loop:
		CPU().ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_StopLoop, this, std::placeholders::_1));
		break;
	case Configuration::StopCondition::Halt:
		break;
	default:
		throw std::domain_error("Unknown stop condition");
	}

	if (m_configuration.allowInput()) {
		ReadingByte.connect(std::bind(&Board::Memory_ReadingByte_Input, this, std::placeholders::_1));
		CPU().ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_Poll, this, std::placeholders::_1));
	}

	if (m_configuration.allowOutput())
		WrittenByte.connect(std::bind(&Board::Memory_WrittenByte_Output, this, std::placeholders::_1));

	m_pollCounter = 0;
	m_pollInterval = m_configuration.getPollInterval();

	CPU().powerOn();
	CPU().PC().word = m_configuration.getStartAddress();
}

void Board::Cpu_ExecutingInstruction_Profile(const EightBit::MOS6502& cpu) {

	const auto pc = CPU().PC();

	//m_profiler.addAddress(pc.word, m_cpu.);
	//m_profiler.addInstruction(m_memory.peek(pc.word));
}

void Board::Cpu_ExecutedInstruction_StopLoop(EightBit::MOS6502& cpu) {

	auto pc = cpu.PC().word;
	if (m_oldPC == pc) {
		CPU().powerOff();
		auto test = peek(0x0200);
		std::cout << std::endl << "** Test=" << std::hex << (int)test;
	} else {
		m_oldPC = pc;
	}
}

void Board::Cpu_ExecutingInstruction_Debug(EightBit::MOS6502& cpu) {

	auto address = cpu.PC().word;
	auto cell = peek(address);

	std::cout << std::hex;
	std::cout << "PC=" << std::setw(4) << std::setfill('0') << address << ":";
	std::cout << "P=" << m_disassembler.dump_Flags(CPU().P()) << ", ";
	std::cout << std::setw(2) << std::setfill('0');
	std::cout << "A=" << (int)CPU().A() << ", ";
	std::cout << "X=" << (int)CPU().X() << ", ";
	std::cout << "Y=" << (int)CPU().Y() << ", ";
	std::cout << "S=" << (int)CPU().S() << "\t";

	std::cout << m_disassembler.disassemble(address);

	std::cout << "\n";
}

void Board::Memory_ReadingByte_Input(EightBit::EventArgs) {
	if (ADDRESS().word == m_configuration.getInputAddress()) {
		if (DATA() != 0)
			write(0);
	}
}

void Board::Memory_WrittenByte_Output(EightBit::EventArgs) {
	if (ADDRESS().word == m_configuration.getOutputAddress()) {
#ifdef _MSC_VER
		_putch(DATA());
#endif
	}
}

void Board::Cpu_ExecutedInstruction_Poll(const EightBit::MOS6502& cpu) {
	if (++m_pollCounter == m_pollInterval) {
		m_pollCounter = 0;
		pollKeyboard();
	}
}

void Board::pollKeyboard() {
#ifdef _MSC_VER
	if (_kbhit())
		poke(m_configuration.getInputAddress(), _getch());
#endif
}