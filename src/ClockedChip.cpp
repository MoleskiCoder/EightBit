#include "stdafx.h"
#include "../inc/ClockedChip.h"

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
