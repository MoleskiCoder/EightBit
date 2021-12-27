#include "stdafx.h"
#include "../inc/ClockedChip.h"

EightBit::ClockedChip::ClockedChip(const ClockedChip& rhs)
: Chip(rhs) {
	m_cycles = rhs.m_cycles;
}

void EightBit::ClockedChip::tick(const int extra) {
	for (int i = 0; i < extra; ++i)
		tick();
}

void EightBit::ClockedChip::tick() {
	++m_cycles;
	Ticked.fire();
}

void EightBit::ClockedChip::resetCycles() noexcept {
	m_cycles = 0;
}

bool EightBit::ClockedChip::operator==(const EightBit::ClockedChip& rhs) const {
	return
		Device::operator==(rhs)
		&& cycles() == rhs.cycles();
}
