#include "stdafx.h"
#include "../inc/InputOutput.h"

#include <cassert>
#include <stdexcept>

#include "../inc/Register.h"

uint16_t EightBit::InputOutput::size() const noexcept {
	return 0x100;
}

uint8_t EightBit::InputOutput::peek(uint16_t) const noexcept {
	assert(false && "Peek operation not allowed.");
	return 0xff;
}

uint8_t& EightBit::InputOutput::reference(uint16_t address) noexcept {
	const auto port = register16_t(address).low;
	switch (accessType()) {
	case AccessType::Reading:
		return m_input.reference(port);
	case AccessType::Writing:
		return m_output.reference(port);
	default:
		assert(false && "Unknown I/O access type.");
	}
	return m_delivered;
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

void EightBit::InputOutput::poke(uint16_t, uint8_t) noexcept {
	assert(false && "Poke operation not allowed.");
}
