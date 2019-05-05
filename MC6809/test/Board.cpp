#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration) {}

void Board::raisePOWER() {

	EightBit::Bus::raisePOWER();

	// Get the CPU ready for action
	CPU().raisePOWER();
	CPU().lowerRESET();
	CPU().raiseINT();
	CPU().raiseNMI();
	CPU().raiseFIRQ();
	CPU().raiseHALT();

	// Get the ACIA ready for action
	ADDRESS() = 0b1010000000000000;
	DATA() = EightBit::mc6850::CR0 | EightBit::mc6850::CR1;	// Master reset
	updateAciaPinsWrite();
	ACIA().lower(ACIA().CTS());
	ACIA().raisePOWER();
	accessAcia();
}

void Board::lowerPOWER() {
	ACIA().lowerPOWER();
	CPU().lowerPOWER();
	EightBit::Bus::lowerPOWER();
}

void Board::initialise() {

	// Load our BASIC interpreter
	const auto directory = m_configuration.getRomDirectory() + "\\";
	loadHexFile(directory + "ExBasROM.hex");

	// Catch a byte being transmitted
	ACIA().Transmitting.connect([this] (EightBit::EventArgs&) {
		std::cout << ACIA().TDR();
		ACIA().markTransmitComplete();
	});

	// Marshal data from memory -> ACIA
	WrittenByte.connect([this] (EightBit::EventArgs&) {
		updateAciaPinsWrite();
		if (ACIA().selected()) {
			ACIA().DATA() = DATA();
			accessAcia();
		}
	});

	// Marshal data from ACIA -> memory
	ReadingByte.connect([this] (EightBit::EventArgs&) {
		updateAciaPinsRead();
		if (accessAcia())
			poke(ACIA().DATA());
	});

	// Keyboard wiring, check for input once per frame
	CPU().ExecutedInstruction.connect([this] (EightBit::mc6809& cpu) {
		assert(cpu.cycles() > 0);
		m_frameCycleCount -= cpu.cycles();
		if (m_frameCycleCount < 0) {
			if (_kbhit()) {
				ACIA().RDR() = _getch();
				ACIA().markReceiveStarting();
			}
			m_frameCycleCount = Configuration::FrameCycleInterval;
		}
	});

	if (m_configuration.isDebugMode()) {
		// MC6809 disassembly wiring
		CPU().ExecutingInstruction.connect([this] (EightBit::mc6809& cpu) {
			m_disassembleAt = cpu.PC();
			m_ignoreDisassembly = m_disassembler.ignore();
		});
		CPU().ExecutedInstruction.connect([this] (EightBit::mc6809&) {
			if (!m_ignoreDisassembly)
				std::cout << m_disassembler.trace(m_disassembleAt) << "	" << ACIA().dumpStatus() << std::endl;
		});
	}

	if (m_configuration.terminatesEarly()) {
		// Early termination condition for CPU timing code
		CPU().ExecutedInstruction.connect([this] (EightBit::mc6809& cpu) {
			assert(cpu.cycles() > 0);
			m_totalCycleCount += cpu.cycles();
			if (m_totalCycleCount > Configuration::TerminationCycles)
				lowerPOWER();
		});
	}
}

EightBit::MemoryMapping Board::mapping(uint16_t address) {

	if (address < 0x8000)
		return { m_ram, 0x0000, EightBit::Chip::Mask16, EightBit::MemoryMapping::AccessLevel::ReadWrite };

	if (address < 0xa000)
		return { m_unused2000, 0x8000, EightBit::Chip::Mask16, EightBit::MemoryMapping::AccessLevel::ReadOnly };

	if (address < 0xc000)
		return { m_io, 0xa000, EightBit::Chip::Mask16, EightBit::MemoryMapping::AccessLevel::ReadWrite };

	return { m_rom, 0xc000, EightBit::Chip::Mask16, EightBit::MemoryMapping::AccessLevel::ReadOnly };
}

void Board::updateAciaPins() {
	ACIA().DATA() = DATA();
	ADDRESS().word & EightBit::Chip::Bit0 ? ACIA().raise(ACIA().RS()) : ACIA().lower(ACIA().RS());
	ADDRESS().word & EightBit::Chip::Bit15 ? ACIA().raise(ACIA().CS0()) : ACIA().lower(ACIA().CS0());
	ADDRESS().word & EightBit::Chip::Bit13 ? ACIA().raise(ACIA().CS1()) : ACIA().lower(ACIA().CS1());
	ADDRESS().word & EightBit::Chip::Bit14 ? ACIA().raise(ACIA().CS2()) : ACIA().lower(ACIA().CS2());
}

bool Board::accessAcia() {
	ACIA().raise(ACIA().E());
	ACIA().tick();
	ACIA().lower(ACIA().E());
	return ACIA().activated();
}
