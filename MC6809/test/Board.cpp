#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration) {}

void Board::powerOn() {

	EightBit::Bus::powerOn();

	// Get the CPU ready for action
	CPU().powerOn();
	CPU().raise(CPU().NMI());
	CPU().raise(CPU().FIRQ());
	CPU().reset();

	// Get the ACIA ready for action
	ADDRESS() = 0b1010000000000000;
	ACIA().DATA() = EightBit::mc6850::CR0 | EightBit::mc6850::CR1;	// Master reset
	updateAciaPinsWrite();
	ACIA().lower(ACIA().CTS());
	ACIA().powerOn();
	accessAcia();
}

void Board::powerOff() {
	ACIA().powerOff();
	CPU().powerOff();
	EightBit::Bus::powerOff();
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
		if (accessAcia()) {
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
				powerOff();
		});
	}
}

EightBit::MemoryMapping Board::mapping(uint16_t address) {

	if (address < 0x8000)
		return { m_ram, 0x0000, EightBit::Chip::Mask16, EightBit::MemoryMapping::ReadWrite };

	if (address < 0xa000)
		return { m_unused2000, 0x8000, EightBit::Chip::Mask16, EightBit::MemoryMapping::ReadOnly };

	if (address < 0xc000)
		return { m_io, 0xa000, EightBit::Chip::Mask16, EightBit::MemoryMapping::ReadWrite };

	return { m_rom, 0xc000, EightBit::Chip::Mask16, EightBit::MemoryMapping::ReadOnly };
}

void Board::updateAciaPins(const EightBit::Chip::PinLevel rw) {
	ACIA().RW() = rw;
	ACIA().DATA() = DATA();
	ACIA().match(ACIA().RS(), ADDRESS().word & EightBit::Chip::Bit0);
	ACIA().match(ACIA().CS0(), ADDRESS().word & EightBit::Chip::Bit15);
	ACIA().match(ACIA().CS1(), ADDRESS().word & EightBit::Chip::Bit13);
	ACIA().match(ACIA().CS2(), ADDRESS().word & EightBit::Chip::Bit14);
}

bool Board::accessAcia() {
	ACIA().raise(ACIA().E());
	const bool accessed = ACIA().tick();
	ACIA().lower(ACIA().E());
	return accessed;
}
