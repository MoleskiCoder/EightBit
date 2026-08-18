#include "stdafx.h"
#include "../inc/ClockedChip.h"

EightBit::ClockedChip::ClockedChip(const ClockedChip& rhs) noexcept
: base(rhs) {
	m_cycles = rhs.m_cycles;
}

bool EightBit::ClockedChip::operator==(const EightBit::ClockedChip& rhs) const noexcept {
	return
		base::operator==(rhs)
		&& cycles() == rhs.cycles();
}
