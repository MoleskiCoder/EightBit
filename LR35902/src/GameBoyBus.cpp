#include "stdafx.h"
#include "GameBoyBus.h"

EightBit::GameBoy::Bus::Bus()
: m_bootRom(0x100),
  m_gameRomBanks(1),
  m_videoRam(0x2000),
  m_ramBanks(0),
  m_lowInternalRam(0x2000),
  m_oamRam(0xa0),
  m_ioPorts(0x80),
  m_highInternalRam(0x80),
  m_disableBootRom(false),
  m_disableGameRom(false),
  m_rom(false),
  m_banked(false),
  m_ram(false),
  m_battery(false),
  m_higherRomBank(true),
  m_ramBankSwitching(false),
  m_romBank(1),
  m_ramBank(0),
  m_timerCounter(0),
  m_timerRate(0),
  m_dmaTransferActive(false),
  m_scanP15(false),
  m_scanP14(false),
  m_p15(true),
  m_p14(true),
  m_p13(true),
  m_p12(true),
  m_p11(true),
  m_p10(true) {
	ReadingByte.connect(std::bind(&GameBoy::Bus::Bus_ReadingByte, this, std::placeholders::_1));
	WrittenByte.connect(std::bind(&GameBoy::Bus::Bus_WrittenByte, this, std::placeholders::_1));
	m_divCounter.word = 0xabcc;
	m_dmaAddress.word = 0;
}

void EightBit::GameBoy::Bus::reset() {

	pokeRegister(NR52, 0xf1);
	m_soundChannelEnabled[0] = true;
	m_soundChannelEnabled[1] = false;
	m_soundChannelEnabled[2] = false;
	m_soundChannelEnabled[3] = false;
	m_soundEnabled = true;

	pokeRegister(LCDC, DisplayBackground | BackgroundCharacterDataSelection | LcdEnable);
	m_divCounter.word = 0xabcc;
	m_timerCounter = 0;
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

void EightBit::GameBoy::Bus::Bus_ReadingByte(const uint16_t address) {
	auto io = ((address >= BASE) && (address < 0xff80)) || (address == 0xffff);
	if (io) {
		auto ioRegister = address - BASE;
		switch (ioRegister) {

		// Port/Mode Registers
		case P1: {
				auto p14 = m_scanP14 && !m_p14;
				auto p15 = m_scanP15 && !m_p15;
				auto live = p14 || p15;
				auto p10 = live && !m_p10;
				auto p11 = live && !m_p11;
				auto p12 = live && !m_p12;
				auto p13 = live && !m_p13;
				pokeRegister(P1,
					   (int)!p10
					| ((int)!p11 << 1)
					| ((int)!p12 << 2)
					| ((int)!p13 << 3)
					| Processor::Bit4 | Processor::Bit5
					| Processor::Bit6 | Processor::Bit7);
			}
			break;
		case SB:
			break;
		case SC:
			mask(Processor::Bit7 | Processor::Bit0);
			break;

		// Timer control
		case DIV:
		case TIMA:
		case TMA:
			break;
		case TAC:
			mask(Processor::Mask3);
			break;

		// Interrupt Flags
		case IF:
			mask(Processor::Mask5);
			break;
		case IE:
			// Only the  bottom 5 bits are used,
			// but all are available for use.
			break;

		// Sound Registers
		case NR10:
			mask(Processor::Mask7);
			break;
		case NR11:
		case NR12:
		case NR13:
		case NR14:
		case NR21:
		case NR22:
		case NR23:
		case NR24:
			break;
		case NR30:
			mask(Processor::Bit7);
			break;
		case NR31:
			break;
		case NR32:
			mask(Processor::Bit6 | Processor::Bit5);
			break;
		case NR33:
		case NR34:
			break;
		case NR41:
			mask(Processor::Mask6);
			break;
		case NR42:
		case NR43:
			break;
		case NR44:
			mask(Processor::Bit6 | Processor::Bit7);
			break;
		case NR50:
		case NR51:
			break;
		case NR52:
			pokeRegister(NR52,
				  (int)m_soundChannelEnabled[0]
				| (m_soundChannelEnabled[1] << 1)
				| (m_soundChannelEnabled[2] << 2)
				| (m_soundChannelEnabled[3] << 3)
				| Processor::Bit4 | Processor::Bit5 | Processor::Bit6
				| (m_soundEnabled << 7));
			break;

		// LCD Display Registers
		case LCDC:
			break;
		case STAT:
			mask(Processor::Mask7);
			break;
		case SCY:
		case SCX:
		case LY:
		case LYC:
		case DMA:
		case BGP:
		case OBP0:
		case OBP1:
		case WY:
		case WX:
			break;

		default:
			mask(0);
			break;
		}
	}
}

void EightBit::GameBoy::Bus::Bus_WrittenByte(const uint16_t address) {

	const auto value = DATA();

	auto handled = false;

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
			m_romBank = value & Processor::Mask5;
			handled = true;
		}
		break;
	case 0x4000:
		// Register 2: ROM bank selection
		if (m_banked) {
			assert(false);
		}
	case 0x6000:
		// Register 3: ROM/RAM change
		if (m_banked) {
			switch (value & Processor::Mask1) {
			case 0:
				m_higherRomBank = true;
				m_ramBankSwitching = false;
				break;
			case 1:
				m_higherRomBank = false;
				m_ramBankSwitching = true;
				break;
			default:
				__assume(0);
			}
			handled = true;
		}
		break;
	}

	if (!handled) {
		switch (address) {

		case BASE + P1:
			m_scanP14 = (value & Processor::Bit4) == 0;
			m_scanP15 = (value & Processor::Bit5) == 0;
			break;

		case BASE + SB:		// R/W
		case BASE + SC:		// R/W
			break;

		case BASE + DIV:	// R/W
			EightBit::Bus::reference() = 0;
			m_timerCounter = m_divCounter.word = 0;
			break;
		case BASE + TIMA:	// R/W
		case BASE + TMA:	// R/W
			break;
		case BASE + TAC:	// R/W
			m_timerRate = timerClockTicks();
			break;

		case BASE + IF:		// R/W
			break;

		case BASE + NR10:
		case BASE + NR11:
		case BASE + NR12:
		case BASE + NR13:
		case BASE + NR14:
		case BASE + NR21:
		case BASE + NR22:
		case BASE + NR23:
		case BASE + NR24:
			break;
		case BASE + NR30:
			break;
		case BASE + NR31:
		case BASE + NR32:
		case BASE + NR33:
		case BASE + NR34:
		case BASE + NR41:
		case BASE + NR42:
		case BASE + NR43:
		case BASE + NR44:
		case BASE + NR50:
		case BASE + NR51:
			break;
		case BASE + NR52:
			m_soundEnabled = (value & Processor::Bit7) != 0;
			break;

		case BASE + LCDC:
		case BASE + STAT:
		case BASE + SCY:
		case BASE + SCX:
			break;
		case BASE + DMA:
			m_dmaAddress.high = value;
			m_dmaAddress.low = 0;
			m_dmaTransferActive = true;
			break;
		case BASE + LY:		// R/O
			EightBit::Bus::reference() = 0;
			break;
		case BASE + BGP:
		case BASE + OBP0:
		case BASE + OBP1:
		case BASE + WY:
		case BASE + WX:
			break;

		case BASE + BOOT_DISABLE:
			m_disableBootRom = value != 0;
			break;

		default:
			if ((address >= (BASE + WPRAM_START)) && (address <= (BASE + WPRAM_END)))
				;	// Wave form data
			else if ((address > BASE) && (address < (BASE + 0x4c)))
				assert(false);
		}
	}
}

void EightBit::GameBoy::Bus::checkTimers(int cycles) {
	incrementDIV(cycles);
	checkTimer(cycles);
}

void EightBit::GameBoy::Bus::checkTimer(int cycles) {
	if (timerEnabled()) {
		m_timerCounter -= cycles;
		if (m_timerCounter <= 0) {
			m_timerCounter += m_timerRate;
			incrementTIMA();
		}
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
		int gameRomBanks = -1;
		int romSizeSpecification = m_gameRomBanks[0].peek(0x148);
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
			gameRomBanks = 1 << (romSizeSpecification + 1);
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
