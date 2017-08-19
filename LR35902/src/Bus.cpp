#include "stdafx.h"
#include "Bus.h"

EightBit::Bus::Bus()
: Memory(0xffff) {
	WrittenByte.connect(std::bind(&Bus::Bus_WrittenByte, this, std::placeholders::_1));
}

void EightBit::Bus::reset() {
	writeRegister(NR52, 0xf1);
	writeRegister(LCDC, 0x91);
}

void EightBit::Bus::clear() {
	Memory::clear();
	m_boot.fill(0);
}

void EightBit::Bus::loadBootRom(const std::string& path) {
	auto size = loadMemory(path, 0);
	if (size != 0x100)
		throw std::runtime_error("Incorrectly sized boot ROM");
	std::copy_n(m_bus.cbegin(), size, m_boot.begin());
}

uint8_t& EightBit::Bus::reference() {
	auto effective = effectiveAddress(ADDRESS().word);
	if (isBootRom(effective))
		return placeDATA(m_boot[effective]);
	return Memory::reference();
}

uint8_t EightBit::Bus::peek(uint16_t address) const {
	auto effective = effectiveAddress(address);
	if (isBootRom(effective))
		return m_boot[effective];
	return m_bus[effective];
}

void EightBit::Bus::Bus_WrittenByte(const AddressEventArgs& e) {
	switch (e.getAddress()) {
	case BASE + TAC:
		m_timerRate = timerClockTicks();
		break;
	case BASE + BOOT_DISABLE:
		m_disableBootRom = e.getCell() != 0;
		break;
	}
}