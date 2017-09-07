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
  m_timerRate(0) {
	WrittenByte.connect(std::bind(&GameBoy::Bus::Bus_WrittenByte, this, std::placeholders::_1));
	m_divCounter.word = 0xabcc;
}

void EightBit::GameBoy::Bus::reset() {
	pokeRegister(NR52, 0xf1);
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

void EightBit::GameBoy::Bus::Bus_WrittenByte(const AddressEventArgs& e) {

	const auto address = e.getAddress();
	const auto value = e.getCell();

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
		case BASE + NR22:
		case BASE + NR24:
		case BASE + NR30:
		case BASE + NR42:
		case BASE + NR44:
		case BASE + NR50:
		case BASE + NR51:
		case BASE + NR52:
			break;

		case BASE + LCDC:
		case BASE + STAT:
		case BASE + SCY:
		case BASE + SCX:
		case BASE + DMA:
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
			if ((address > BASE) && (address < (BASE + 0x4c)))
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
}
