#include "stdafx.h"
#include "Bus.h"

EightBit::Bus::Bus()
: Memory(0xffff),
  m_disableBootRom(false),
  m_disableGameRom(false),
  m_rom(false),
  m_ram(false),
  m_battery(false),
  m_higherRomBank(true),
  m_ramBankSwitching(false),
  m_romBank(1),
  m_ramBank(0),
  m_divCounter(256),
  m_timerCounter(0),
  m_timerRate(0) {
	m_bootRom.resize(0x100);
	m_gameRom.resize(0x10000);
	WrittenByte.connect(std::bind(&Bus::Bus_WrittenByte, this, std::placeholders::_1));
}

void EightBit::Bus::reset() {
	writeRegister(NR52, 0xf1);
	writeRegister(LCDC, 0x91);
}

void EightBit::Bus::clear() {
	Memory::clear();
	std::fill(m_bootRom.begin(), m_bootRom.end(), 0);
	std::fill(m_gameRom.begin(), m_gameRom.end(), 0);
}

void EightBit::Bus::loadBootRom(const std::string& path) {
	loadBinary(path, m_bootRom, 0, 0x100);
}

void EightBit::Bus::loadGameRom(const std::string& path) {
	loadBinary(path, m_gameRom);
	validateCartridgeType();
}

uint8_t& EightBit::Bus::reference(uint16_t address, bool& rom) {
	rom = true;
	auto effective = effectiveAddress(address);
	if ((effective < 0x100) && bootRomEnabled())
		return m_bootRom[effective];
	if ((effective < 0x4000) && gameRomEnabled())
		return m_gameRom[effective];
	if ((effective < 0x8000) && gameRomEnabled())
		return m_gameRom[(effective - RomPageSize) + (m_romBank * RomPageSize)];
	rom = false;
	return m_bus[effective];
}

void EightBit::Bus::Bus_WrittenByte(const AddressEventArgs& e) {

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
		case BASE + TAC:
			m_timerRate = timerClockTicks();
			break;
		case BASE + BOOT_DISABLE:
			m_disableBootRom = value != 0;
			break;
		case BASE + DIV:
			Memory::reference() = 0;
			m_timerCounter = 0;
			break;
		}
	}
}

void EightBit::Bus::checkTimers(int cycles) {
	checkDiv(cycles);
	checkTimer(cycles);
}

void EightBit::Bus::checkDiv(int cycles) {
	m_divCounter -= cycles;
	if (m_divCounter <= 0) {
		m_divCounter = 256;
		incrementDIV();
	}
}

void EightBit::Bus::checkTimer(int cycles) {
	if (timerEnabled()) {
		m_timerCounter -= cycles;
		if (m_timerCounter <= 0) {
			m_timerCounter = m_timerRate;
			incrementTIMA();
		}
	}
}

void EightBit::Bus::validateCartridgeType() {

	m_rom = m_banked = m_ram = m_battery = false;

	switch (m_gameRom[0x147]) {
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
