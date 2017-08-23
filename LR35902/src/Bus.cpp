#include "stdafx.h"
#include "Bus.h"

EightBit::Bus::Bus()
: Memory(0xffff),
  m_disableBootRom(false),
  m_disableGameRom(false),
  m_mbc1(false),
  m_rom(false),
  m_ram(false),
  m_battery(false),
  m_mbc1Mode(SixteenEight),
  m_romBank(0),
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

uint8_t& EightBit::Bus::reference() {
	auto effective = effectiveAddress(ADDRESS().word);
	if ((effective < 0x100) && bootRomEnabled())
		return placeDATA(m_bootRom[effective]);
	if ((effective < 0x4000) && gameRomEnabled())
		return placeDATA(m_gameRom[effective]);
	if ((effective < 0x8000) && gameRomEnabled())
		return placeDATA(m_gameRom[effective + (m_romBank * 0x4000)]);
	return Memory::reference();
}

uint8_t EightBit::Bus::peek(uint16_t address) const {
	auto effective = effectiveAddress(address);
	if ((effective < 0x100) && bootRomEnabled())
		return m_bootRom[effective];
	if ((effective < 0x4000) && gameRomEnabled())
		return m_gameRom[effective];
	if ((effective < 0x8000) && gameRomEnabled())
		return m_gameRom[effective + (m_romBank * 0x4000)];
	return m_bus[effective];
}

void EightBit::Bus::poke(uint16_t address, uint8_t value) {
	auto effective = effectiveAddress(address);
	if ((effective < 0x100) && bootRomEnabled())
		m_bootRom[effective] = value;
	if ((effective < 0x4000) && gameRomEnabled())
		m_gameRom[effective] = value;
	if ((effective < 0x8000) && gameRomEnabled())
		m_gameRom[effective + (m_romBank * 0x4000)] = value;
	m_bus[effective] = value;
}

void EightBit::Bus::Bus_WrittenByte(const AddressEventArgs& e) {

	const auto address = e.getAddress();
	const auto value = e.getCell();

	switch (address & 0xe000) {
	case 0x2000:
		m_romBank = value & Processor::Mask5;
		break;
	case 0x6000:
		m_mbc1Mode = value & Processor::Mask1 ? FourThirtyTwo : SixteenEight;
		break;
	default:
		switch (address) {
		case BASE + TAC:
			m_timerRate = timerClockTicks();
			break;
		case BASE + BOOT_DISABLE:
			m_disableBootRom = value != 0;
			break;
		case BASE + DIV:
			reference() = 0;
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

	m_rom = m_mbc1 = m_ram = m_battery = false;

	switch (m_gameRom[0x147]) {
	case ROM:
		m_rom = true;
		break;
	case ROM_MBC1:
		m_rom = m_mbc1 = true;
		break;
	case ROM_MBC1_RAM:
		m_rom = m_mbc1 = m_ram = true;
		break;
	case ROM_MBC1_RAM_BATTERY:
		m_rom = m_mbc1 = m_ram = m_battery = true;
		break;
	default:
		throw std::domain_error("Unhandled cartridge ROM type");
	}
}
