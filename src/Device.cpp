#include "stdafx.h"
#include "../inc/Device.h"

DEFINE_PIN_LEVEL_CHANGERS(POWER, Device);

EightBit::Device::Device(const Device& rhs) noexcept {
	POWER() = rhs.POWER();
}

bool EightBit::Device::operator==(const EightBit::Device& rhs) const noexcept {
	return POWER() == rhs.POWER();
}
