#pragma once

#include <cstdint>

class AddressEventArgs
{
private:
	uint16_t m_address;
	uint8_t m_cell;

public:
	AddressEventArgs(uint16_t address, uint8_t cell)
	: m_address(address), m_cell(cell) {}

	uint16_t getAddress() const	{ return m_address;	}
	uint8_t getCell() const		{ return m_cell;	}
};
