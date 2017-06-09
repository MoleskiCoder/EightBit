#include "stdafx.h"
#include "Bus.h"

Bus::Bus()
: Memory(0xffff) {
}

void Bus::reset() {
	REG(NR52) = 0xf1;
	REG(LCDC) = 0x91;
}

void Bus::loadBootRom(const std::string& path) {
	auto size = loadMemory(path, 0);
	if (size != 0x100)
		throw std::runtime_error("Incorrectly sized boot ROM");
	std::copy_n(m_bus.cbegin(), size, m_boot.begin());
}

uint8_t& Bus::reference() {
	auto effective = effectiveAddress(ADDRESS().word);
	if (isBootRom(effective))
		return placeDATA(m_boot[effective]);
	return Memory::reference();
}

uint8_t Bus::peek(uint16_t address) const {
	auto effective = effectiveAddress(address);
	if (isBootRom(effective))
		return m_boot[effective];
	return m_bus[effective];
}