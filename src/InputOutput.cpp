#include "stdafx.h"
#include "InputOutput.h"

#include <stdexcept>

#include "Register.h"

size_t EightBit::InputOutput::size() const noexcept {
	return 0x100;
}

uint8_t EightBit::InputOutput::peek(uint16_t) const {
	throw std::logic_error("Peek operation not allowed.");
}

uint8_t& EightBit::InputOutput::reference(uint16_t address) {
	const auto port = register16_t(address).low;
	switch (accessType()) {
	case AccessType::Reading:
		return m_input.reference(port);
	case AccessType::Writing:
		return m_output.reference(port);
	default:
		throw std::logic_error("Unknown I/O access type.");
	}
}

int EightBit::InputOutput::load(std::ifstream&, int, int, int) {
	throw std::logic_error("load operation not allowed.");
}

int EightBit::InputOutput::load(std::string, int, int, int) {
	throw std::logic_error("load operation not allowed.");
}

int EightBit::InputOutput::load(const std::vector<uint8_t>&, int, int, int) {
	throw std::logic_error("load operation not allowed.");
}

void EightBit::InputOutput::poke(uint16_t, uint8_t) {
	throw std::logic_error("Poke operation not allowed.");
}
