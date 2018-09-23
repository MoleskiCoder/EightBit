#include "stdafx.h"
#include "Chip.h"

void EightBit::Chip::powerOn() {
	raise(POWER());
}