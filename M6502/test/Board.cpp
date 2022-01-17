#include "stdafx.h"
#include "Board.h"
#include "Disassembly.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <assert.h>

Board::Board(const Configuration& configuration)
: m_configuration(configuration) {}

void Board::raisePOWER() noexcept {
	EightBit::Bus::raisePOWER();
	CPU().raisePOWER();
	CPU().raiseRESET();
	CPU().raiseINT();
	CPU().raiseNMI();
	CPU().raiseSO();
	CPU().raiseRDY();
}

void Board::lowerPOWER() noexcept {
	CPU().lowerPOWER();
	EightBit::Bus::lowerPOWER();
}

void Board::initialise() {

	auto programFilename = m_configuration.getProgram();
	auto programPath = m_configuration.getRomDirectory() + "/" + m_configuration.getProgram();
	auto loadAddress = m_configuration.getLoadAddress();
	m_ram.load(programPath, loadAddress.word);

	// Disassembly output
	if (m_configuration.isDebugMode())
		CPU().ExecutingInstruction.connect([this] (EightBit::MOS6502& cpu) {
			const auto address = cpu.PC();
			const auto cell = peek(address);

			std::cout << std::hex;
			std::cout << "PC=" << std::setw(4) << std::setfill('0') << address << ":";
			std::cout << "P=" << m_disassembler.dump_Flags(CPU().P()) << ", ";
			std::cout << std::setw(2) << std::setfill('0');
			std::cout << "A=" << (int)CPU().A() << ", ";
			std::cout << "X=" << (int)CPU().X() << ", ";
			std::cout << "Y=" << (int)CPU().Y() << ", ";
			std::cout << "S=" << (int)CPU().S() << "\t";

			std::cout << m_disassembler.disassemble(address.word);

			std::cout << "\n";
		});

	// Loop detected stop condition
	CPU().ExecutedInstruction.connect([this] (EightBit::MOS6502& cpu) {
		const auto pc = cpu.PC();
		if (m_oldPC != pc) {
			m_oldPC = pc;
		} else {
			lowerPOWER();
			auto test = peek(0x0200);
			std::cout << std::endl << "** Test=" << std::hex << (int)test;
		}
	});

	poke(0x00, 0x4c);
	CPU().pokeWord(1, m_configuration.getStartAddress());
}
