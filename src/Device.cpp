#include "stdafx.h"
#include "../inc/Device.h"

DEFINE_PIN_LEVEL_CHANGERS(POWER, Device);

bool EightBit::Device::operator==(const EightBit::Device& rhs) const {
	return POWER() == rhs.POWER();
}
