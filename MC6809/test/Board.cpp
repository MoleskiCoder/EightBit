#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::mc6809(*this)),
  m_disassembler(*this, m_cpu) {
	std::vector<uint8_t> content(m_unused2000.size());
	std::fill(content.begin(), content.end(), 0xff);
	m_unused2000.load(content);
}

void Board::initialise() {

	// Load our BASIC interpreter
	const auto directory = m_configuration.getRomDirectory() + "\\";
	loadHexFile(directory + "ExBasROM.hex");

	// Get the CPU ready for action
	CPU().powerOn();
	CPU().raise(CPU().NMI());
	CPU().raise(CPU().FIRQ());
	CPU().reset();

	// Get the ACIA ready for action
	ADDRESS() = 0b1010000000000000;
	ACIA().DATA() = EightBit::mc6850::CR0 | EightBit::mc6850::CR1;	// Master reset
	updateAciaPins(EightBit::Chip::PinLevel::Low);
	ACIA().powerOn();
	ACIA().access();

	// Once the reset has completed, we can wire the ACIA event handlers...
	ACIA().Transmitting.connect(std::bind(&Board::Acia_Transmitting, this, std::placeholders::_1));

	// Wire bus events...
	WrittenByte.connect(std::bind(&Board::Bus_WrittenByte_Acia, this, std::placeholders::_1));
	ReadingByte.connect(std::bind(&Board::Bus_ReadingByte_Acia, this, std::placeholders::_1));

	// Wire CPU events
	CPU().ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_Acia, this, std::placeholders::_1));
	if (m_configuration.isDebugMode()) {
		CPU().ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
		CPU().ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_Debug, this, std::placeholders::_1));
	}
}

void Board::Cpu_ExecutingInstruction_Debug(EightBit::mc6809&) {
	m_disassembleAt = CPU().PC();
	m_ignoreDisassembly = m_disassembler.ignore();
}

void Board::Cpu_ExecutedInstruction_Debug(EightBit::mc6809&) {
	if (!m_ignoreDisassembly)
		std::cout << m_disassembler.trace(m_disassembleAt) << "	" << ACIA().dumpStatus() << std::endl;
}

EightBit::MemoryMapping Board::mapping(uint16_t address) {

	if (address < 0x8000)
		return { m_ram, 0x0000, EightBit::MemoryMapping::ReadWrite };

	if (address < 0xa000)
		return { m_unused2000, 0x8000, EightBit::MemoryMapping::ReadOnly };

	if (address < 0xc000)
		return { m_io, 0xa000, EightBit::MemoryMapping::ReadWrite };

	return { m_rom, 0xc000, EightBit::MemoryMapping::ReadOnly };
}

void Board::Bus_WrittenByte_Acia(EightBit::EventArgs&) {
	updateAciaPins(EightBit::Chip::Low);
	if (ACIA().selected()) {
		ACIA().DATA() = peek(ADDRESS());
		ACIA().access();
	}
}

void Board::Bus_ReadingByte_Acia(EightBit::EventArgs&) {
	updateAciaPins(EightBit::Chip::High);
	if (ACIA().access())
		poke(ADDRESS(), ACIA().DATA());
}

void Board::updateAciaPins(const EightBit::Chip::PinLevel rw) {
	ACIA().RW() = rw;
	ACIA().DATA() = DATA();
	ADDRESS().word & EightBit::Chip::Bit0 ? ACIA().raise(ACIA().RS()) : ACIA().lower(ACIA().RS());
	ADDRESS().word & EightBit::Chip::Bit15 ? ACIA().raise(ACIA().CS0()) : ACIA().lower(ACIA().CS0());
	ADDRESS().word & EightBit::Chip::Bit13 ? ACIA().raise(ACIA().CS1()) : ACIA().lower(ACIA().CS1());
	ADDRESS().word & EightBit::Chip::Bit14 ? ACIA().raise(ACIA().CS2()) : ACIA().lower(ACIA().CS2());
}

void Board::Cpu_ExecutedInstruction_Acia(EightBit::mc6809&) {
	const auto cycles = CPU().cycles();
	m_totalCycleCount += cycles;
	if (m_totalCycleCount < TerminationCycles) {
		m_frameCycleCount -= cycles;
		if (m_frameCycleCount < 0) {
			if (_kbhit()) {
				ACIA().RDR() = _getch();
				ACIA().markReceiveStarting();
			}
			m_frameCycleCount = FrameCycleInterval;
		}
	} else {
		CPU().powerOff();
	}
}

void Board::Acia_Transmitting(EightBit::EventArgs&) {
	std::cout << ACIA().TDR();
	ACIA().markTransmitComplete();
}