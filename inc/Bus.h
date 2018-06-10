#pragma once

#include <cstdint>

#include "Signal.h"
#include "Register.h"
#include "EventArgs.h"

namespace EightBit {
	class Bus {
	public:
		virtual ~Bus() = default;

		Signal<EventArgs> WritingByte;
		Signal<EventArgs> WrittenByte;

		Signal<EventArgs> ReadingByte;
		Signal<EventArgs> ReadByte;

		register16_t& ADDRESS() { return m_address; }
		register16_t ADDRESS() const { return m_address; }
		uint8_t& DATA() { return m_data; }
		uint8_t DATA() const { return m_data; }

		uint8_t peek() const;
		uint8_t peek(uint16_t address) const;
		void poke(uint8_t value);
		void poke(uint16_t address, uint8_t value);

		uint16_t peekWord(uint16_t address) const;

		uint8_t read();
		uint8_t read(uint16_t offset);
		uint8_t read(register16_t address);

		void write();
		void write(uint8_t value);
		void write(uint16_t offset, uint8_t value);
		void write(register16_t address, uint8_t value);

	protected:
		virtual uint8_t& reference(uint16_t address) = 0;
		virtual uint8_t reference(uint16_t address) const = 0;

		uint8_t& reference();
		uint8_t reference() const;

	private:
		uint8_t m_data = 0xff;
		register16_t m_address{ { 0xff, 0xff } };
	};
}
