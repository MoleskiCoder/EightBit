#include "stdafx.h"
#include "Device.h"

void EightBit::Device::raisePOWER() {
	raise(POWER());
	RaisedPOWER.fire(EventArgs::empty());
}

void EightBit::Device::lowerPOWER() {
	lower(POWER());
	LoweredPOWER.fire(EventArgs::empty());
}
