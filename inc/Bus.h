#pragma once

#include <cstdint>

#include "Signal.h"
#include "Register.h"

namespace EightBit {
	class Bus {
	public:
		virtual ~Bus() = default;

		Signal<uint16_t> WrittenByte;
		Signal<uint16_t> ReadingByte;

		register16_t& ADDRESS();
		uint8_t& DATA();

		uint8_t& placeDATA(uint8_t value);
		uint8_t& referenceDATA(uint8_t& value);

		uint8_t peek(uint16_t address);
		void poke(uint16_t address, uint8_t value);

		uint16_t peekWord(uint16_t address);

		uint8_t read();
		uint8_t read(uint16_t offset);
		uint8_t read(register16_t address);

		void write(uint8_t value);
		void write(uint16_t offset, uint8_t value);
		void write(register16_t address, uint8_t value);

	protected:
		virtual uint8_t& reference(uint16_t address, bool& rom) = 0;
		uint8_t& reference();

	private:
		uint8_t* m_data = nullptr;
		register16_t m_address{ { 0xff, 0xff } };
		uint8_t m_temporary = 0xff;	// Used to simulate ROM
	};
}
