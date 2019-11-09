#include "stdafx.h"
#include "GameBoyBus.h"
#include "Display.h"

EightBit::GameBoy::Bus::Bus() noexcept
: m_cpu(*this),
  m_ioPorts(*this) {
	WrittenByte.connect(std::bind(&GameBoy::Bus::Bus_WrittenByte, this, std::placeholders::_1));
}

void EightBit::GameBoy::Bus::raisePOWER() {
	EightBit::Bus::raisePOWER();
	CPU().raisePOWER();
	CPU().raiseINT();
	reset();
}

void EightBit::GameBoy::Bus::lowerPOWER() {
	CPU().lowerPOWER();
	EightBit::Bus::lowerPOWER();
}

void EightBit::GameBoy::Bus::reset() {
	IO().reset();
	CPU().lowerRESET();
}

void EightBit::GameBoy::Bus::loadBootRom(const std::string& path) {
	m_bootRom.load(path);
}

void EightBit::GameBoy::Bus::loadGameRom(const std::string& path) {
	const auto bankSize = 0x4000;
	m_gameRomBanks.resize(1);
	const auto size = m_gameRomBanks[0].load(path, 0, 0, bankSize);
	const auto banks = size / bankSize;
	m_gameRomBanks.resize(banks);
	for (int bank = 1; bank < banks; ++bank)
		m_gameRomBanks[bank].load(path, 0, bankSize * bank, bankSize);
	validateCartridgeType();
}

void EightBit::GameBoy::Bus::Bus_WrittenByte(EightBit::EventArgs) {

	const auto address = ADDRESS().word;
	const auto value = DATA();

	switch (address & 0xe000) {
	case 0x0000:
		// Register 0: RAMCS gate data
		if (m_ram) {
			assert(false);
		}
		break;
	case 0x2000:
		// Register 1: ROM bank code
		if (m_banked && m_higherRomBank) {
			assert((address >= 0x2000) && (address < 0x4000));
			assert((value > 0) && (value < 0x20));
			m_romBank = value & Chip::Mask5;
		}
		break;
	case 0x4000:
		// Register 2: ROM bank selection
		if (m_banked) {
			assert(false);
		}
		break;
	case 0x6000:
		// Register 3: ROM/RAM change
		if (m_banked) {
			switch (value & Chip::Mask1) {
			case 0:
				m_higherRomBank = true;
				m_ramBankSwitching = false;
				break;
			case 1:
				m_higherRomBank = false;
				m_ramBankSwitching = true;
				break;
			default:
				UNREACHABLE;
			}
		}
		break;
	}
}

void EightBit::GameBoy::Bus::validateCartridgeType() {

	m_rom = m_banked = m_ram = m_battery = false;

	// ROM type
	switch (m_gameRomBanks[0].peek(0x147)) {
	case ROM:
		m_rom = true;
		break;
	case ROM_MBC1:
		m_rom = m_banked = true;
		break;
	case ROM_MBC1_RAM:
		m_rom = m_banked = m_ram = true;
		break;
	case ROM_MBC1_RAM_BATTERY:
		m_rom = m_banked = m_ram = m_battery = true;
		break;
	default:
		throw std::domain_error("Unhandled cartridge ROM type");
	}

	// ROM size
	{
		size_t gameRomBanks = 0;
		const int romSizeSpecification = m_gameRomBanks[0].peek(0x148);
		switch (romSizeSpecification) {
		case 0x52:
			gameRomBanks = 72;
			break;
		case 0x53:
			gameRomBanks = 80;
			break;
		case 0x54:
			gameRomBanks = 96;
			break;
		default:
			if (romSizeSpecification > 6)
				throw std::domain_error("Invalid ROM size specification");
			gameRomBanks = Chip::bit(romSizeSpecification + 1);
			if (gameRomBanks != m_gameRomBanks.size())
				throw std::domain_error("ROM size specification mismatch");
		}

		// RAM size
		{
			auto ramSizeSpecification = m_gameRomBanks[0].peek(0x149);
			switch (ramSizeSpecification) {
			case 0:
				break;
			case 1:
				m_ramBanks.resize(1);
				m_ramBanks[0] = Ram(2 * 1024);
				break;
			case 2:
				m_ramBanks.resize(1);
				m_ramBanks[0] = Ram(8 * 1024);
				break;
			case 3:
				m_ramBanks.resize(4);
				for (int i = 0; i < 4; ++i)
					m_ramBanks[i] = Ram(8 * 1024);
				break;
			case 4:
				m_ramBanks.resize(16);
				for (int i = 0; i < 16; ++i)
					m_ramBanks[i] = Ram(8 * 1024);
				break;
			default:
				throw std::domain_error("Invalid RAM size specification");
			}
		}
	}
}

EightBit::MemoryMapping EightBit::GameBoy::Bus::mapping(uint16_t address) {

	if ((address < 0x100) && IO().bootRomEnabled())
		return { m_bootRom, 0x0000, 0xffff, MemoryMapping::AccessLevel::ReadOnly };
	if ((address < 0x4000) && gameRomEnabled())
		return { m_gameRomBanks[0], 0x0000, 0xffff, MemoryMapping::AccessLevel::ReadOnly };
	if ((address < 0x8000) && gameRomEnabled())
		return { m_gameRomBanks[m_romBank], 0x4000, 0xffff, MemoryMapping::AccessLevel::ReadOnly };

	if (address < 0xa000)
		return { VRAM(), 0x8000, 0xffff, MemoryMapping::AccessLevel::ReadWrite };
	if (address < 0xc000) {
		if (m_ramBanks.size() == 0)
			return { m_unmapped2000, 0xa000, 0xffff, MemoryMapping::AccessLevel::ReadOnly };
		else
			return { m_ramBanks[m_ramBank], 0xa000, 0xffff, MemoryMapping::AccessLevel::ReadWrite };
	}
	if (address < 0xe000)
		return { m_lowInternalRam, 0xc000, 0xffff, MemoryMapping::AccessLevel::ReadWrite };
	if (address < 0xfe00)
		return { m_lowInternalRam, 0xe000, 0xffff, MemoryMapping::AccessLevel::ReadWrite };	// Low internal RAM mirror
	if (address < 0xfea0)
		return { OAMRAM(), 0xfe00, 0xffff, MemoryMapping::AccessLevel::ReadWrite };
	if (address < IoRegisters::BASE)
		return { m_unmapped60, 0xfea0, 0xffff, MemoryMapping::AccessLevel::ReadOnly };
	if (address < 0xff80)
		return { IO(), IoRegisters::BASE, 0xffff, MemoryMapping::AccessLevel::ReadWrite };
	return { m_highInternalRam, 0xff80, 0xffff, MemoryMapping::AccessLevel::ReadWrite };
}

int EightBit::GameBoy::Bus::runRasterLines() {
	m_enabledLCD = !!(IO().peek(IoRegisters::LCDC) & IoRegisters::LcdEnable);
	IO().resetLY();
	return runRasterLines(Display::RasterHeight);
}

int EightBit::GameBoy::Bus::runRasterLines(int lines) {
	int count = 0;
	int allowed = CyclesPerLine;
	for (int line = 0; line < lines; ++line) {
		auto executed = runRasterLine(allowed);
		count += executed;
		allowed = CyclesPerLine - (executed - CyclesPerLine);
	}
	return count;
}

int EightBit::GameBoy::Bus::runRasterLine(int limit) {

	/*
	A scanline normally takes 456 clocks (912 clocks in double speed
	mode) to complete. A scanline starts in mode 2, then goes to
	mode 3 and, when the LCD controller has finished drawing the
	line (the timings depend on lots of things) it goes to mode 0.
	During lines 144-153 the LCD controller is in mode 1.
	Line 153 takes only a few clocks to complete (the exact
	timings are below). The rest of the clocks of line 153 are
	spent in line 0 in mode 1!

	During mode 0 and mode 1 the CPU can access both VRAM and OAM.
	During mode 2 the CPU can only access VRAM, not OAM.
	During mode 3 OAM and VRAM can't be accessed.
	In GBC mode the CPU can't access Palette RAM(FF69h and FF6Bh)
	during mode 3.
	A scanline normally takes 456 clocks(912 clocks in double speed mode) to complete.
	A scanline starts in mode 2, then goes to mode 3 and , when the LCD controller has
	finished drawing the line(the timings depend on lots of things) it goes to mode 0.
	During lines 144 - 153 the LCD controller is in mode 1.
	Line 153 takes only a few clocks to complete(the exact timings are below).
	The rest of the clocks of line 153 are spent in line 0 in mode 1!
	*/

	int count = 0;
	if (m_enabledLCD) {

		if ((IO().peek(IoRegisters::STAT) & Chip::Bit6) && (IO().peek(IoRegisters::LYC) == IO().peek(IoRegisters::LY)))
			IO().triggerInterrupt(IoRegisters::Interrupts::DisplayControlStatus);

		// Mode 2, OAM unavailable
		IO().updateLcdStatusMode(IoRegisters::LcdStatusMode::SearchingOamRam);
		if (IO().peek(IoRegisters::STAT) & Chip::Bit5)
			IO().triggerInterrupt(IoRegisters::Interrupts::DisplayControlStatus);
		count += CPU().run(80);	// ~19us

		// Mode 3, OAM/VRAM unavailable
		IO().updateLcdStatusMode(IoRegisters::LcdStatusMode::TransferringDataToLcd);
		count += CPU().run(170);	// ~41us

		// Mode 0
		IO().updateLcdStatusMode(IoRegisters::LcdStatusMode::HBlank);
		if (IO().peek(IoRegisters::STAT) & Chip::Bit3)
			IO().triggerInterrupt(IoRegisters::Interrupts::DisplayControlStatus);
		count += CPU().run(limit - count);	// ~48.6us

		IO().incrementLY();
	} else {
		count += CPU().run(CyclesPerLine);
	}

	return count;
}

int EightBit::GameBoy::Bus::runVerticalBlankLines() {
	const auto lines = TotalLineCount - Display::RasterHeight;
	return runVerticalBlankLines(lines);
}

int EightBit::GameBoy::Bus::runVerticalBlankLines(int lines) {

	/*
	Vertical Blank interrupt is triggered when the LCD
	controller enters the VBL screen mode (mode 1, LY=144).
	This happens once per frame, so this interrupt is
	triggered 59.7 times per second. During this period the
	VRAM and OAM can be accessed freely, so it's the best
	time to update graphics (for example, use the OAM DMA to
	update sprites for next frame, or update tiles to make
	animations).
	This period lasts 4560 clocks in normal speed mode and
	9120 clocks in double speed mode. That's exactly the
	time needed to draw 10 scanlines.
	The VBL interrupt isn't triggered when the LCD is
	powered off or on, even when it was on VBL mode.
	It's only triggered when the VBL period starts.
	*/

	if (m_enabledLCD) {
		IO().updateLcdStatusMode(IoRegisters::LcdStatusMode::VBlank);
		if (IO().peek(IoRegisters::STAT) & Chip::Bit4)
			IO().triggerInterrupt(IoRegisters::Interrupts::DisplayControlStatus);
		IO().triggerInterrupt(IoRegisters::Interrupts::VerticalBlank);
	}
	return runRasterLines(lines);
}
