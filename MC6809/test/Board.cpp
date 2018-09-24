#include "stdafx.h"
#include "Board.h"

Board::Board(const Configuration& configuration)
: m_configuration(configuration),
  m_cpu(EightBit::mc6809(*this)),
  m_disassembler(m_cpu) {
	std::vector<uint8_t> content(m_unused2000.size());
	std::fill(content.begin(), content.end(), 0xff);
	m_unused2000.load(content);
}

void Board::initialise() {

	const auto directory = m_configuration.getRomDirectory() + "\\";

	loadHexFile(directory + "ExBasROM.hex");

	if (m_configuration.isDebugMode()) {
		CPU().ExecutingInstruction.connect(std::bind(&Board::Cpu_ExecutingInstruction_Debug, this, std::placeholders::_1));
		CPU().ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_Debug, this, std::placeholders::_1));
	}

	WritingByte.connect(std::bind(&Board::Bus_WritingByte_Acia, this, std::placeholders::_1));
	ReadingByte.connect(std::bind(&Board::Bus_ReadingByte_Acia, this, std::placeholders::_1));
	CPU().ExecutedInstruction.connect(std::bind(&Board::Cpu_ExecutedInstruction_Acia, this, std::placeholders::_1));

	ACIA().Accessing.connect(std::bind(&Board::Acia_Accessing, this, std::placeholders::_1));
	ACIA().Accessed.connect(std::bind(&Board::Acia_Accessed, this, std::placeholders::_1));

	CPU().powerOn();
	CPU().raise(CPU().NMI());
	CPU().raise(CPU().FIRQ());
	CPU().reset();

	ACIA().powerOn();
	ACIA().RW() = EightBit::Chip::PinLevel::Low;	// Write
	ACIA().RS() = EightBit::Chip::PinLevel::Low;	// Registers
	EightBit::Processor::setFlag(ACIA().DATA(), EightBit::mc6850::CR0 | EightBit::mc6850::CR1);
	ACIA().step(1);	// Get the reset out of the way...
}

void Board::Cpu_ExecutingInstruction_Debug(EightBit::mc6809&) {
	m_disassembleAt = CPU().PC();
	m_ignoreDisassembly = m_disassembler.ignore();
}

void Board::Cpu_ExecutedInstruction_Debug(EightBit::mc6809&) {
	if (!m_ignoreDisassembly)
		std::cout << m_disassembler.trace(m_disassembleAt) << std::endl;
}

void Board::Cpu_ExecutedInstruction_die(EightBit::mc6809&) {
	static uint64_t instructions = 0UL;
	if (++instructions > 90000000)
		CPU().powerOff();
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

void Board::Bus_WritingByte_Acia(EightBit::EventArgs&) {
	updateAciaPins(EightBit::Chip::Low);
}

void Board::Bus_ReadingByte_Acia(EightBit::EventArgs&) {
	updateAciaPins(EightBit::Chip::High);
}

void Board::updateAciaPins(const EightBit::Chip::PinLevel rw) {
	ACIA().RW() = rw;
	ACIA().DATA() = DATA();
	ACIA().RS() = ADDRESS().word & EightBit::Chip::Bit0 ? EightBit::Chip::PinLevel::High : EightBit::Chip::PinLevel::Low;
	ACIA().CS0() = ADDRESS().word & EightBit::Chip::Bit15 ? EightBit::Chip::PinLevel::High : EightBit::Chip::PinLevel::Low;
	ACIA().CS1() = ADDRESS().word & EightBit::Chip::Bit13 ? EightBit::Chip::PinLevel::High : EightBit::Chip::PinLevel::Low;
	ACIA().CS2() = ADDRESS().word & EightBit::Chip::Bit14 ? EightBit::Chip::PinLevel::High : EightBit::Chip::PinLevel::Low;
}

void Board::Cpu_ExecutedInstruction_Acia(EightBit::mc6809&) {
	ACIA().step(CPU().cycles());
}

void Board::Acia_Accessing(EightBit::EventArgs&) {
	if (_kbhit()) {
		ACIA().DATA() = _getch();
		ACIA().fillRDR();
	}
}

void Board::Acia_Accessed(EightBit::EventArgs&) {
}
