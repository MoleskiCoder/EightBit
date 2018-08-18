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

		register16_t ADDRESS() const { return m_address; }
		register16_t& ADDRESS() { return m_address; }

		uint8_t DATA() const { return m_data; }
		uint8_t& DATA() { return m_data; }

		uint8_t peek() { return reference(); }
		uint8_t peek(uint16_t address) { return reference(address); }
		void poke(uint8_t value) { reference() = value; }
		void poke(uint16_t address, uint8_t value) { reference(address) = value; }

		uint16_t peekWord(uint16_t address);

		uint8_t read();
		template<class T> uint8_t read(const T address) {
			ADDRESS() = address;
			return read();
		}

		void write();
		void write(uint8_t value);
		template<class T> void write(const T offset, const uint8_t value) {
			ADDRESS() = offset;
			write(value);
		}

	protected:
		virtual uint8_t& reference(uint16_t address) = 0;
		uint8_t& reference() { return reference(ADDRESS().word); }

	private:
		uint8_t m_data = 0xff;
		register16_t m_address = 0xffff;
	};
}
