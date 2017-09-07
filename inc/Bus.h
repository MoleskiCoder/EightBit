#pragma once

#include <cstdint>

#include "Signal.h"
#include "AddressEventArgs.h"
#include "Register.h"

namespace EightBit {
	class Bus {
	public:
		Bus()
		: m_temporary(0xff), m_data(nullptr) {
			m_address.word = 0xffff;
		}

		Signal<AddressEventArgs> WrittenByte;
		Signal<AddressEventArgs> ReadByte;

		register16_t& ADDRESS() { return m_address; }
		uint8_t& DATA() { return *m_data; }

		uint8_t& placeDATA(uint8_t value) {
			m_temporary = value;
			m_data = &m_temporary;
			return DATA();
		}

		uint8_t& referenceDATA(uint8_t& value) {
			m_data = &value;
			return DATA();
		}

		uint8_t peek(uint16_t address) {
			bool rom;
			return reference(address, rom);
		}

		void poke(uint16_t address, uint8_t value) {
			bool rom;
			reference(address, rom) = value;
		}

		uint16_t peekWord(uint16_t address) {
			register16_t returned;
			returned.low = peek(address);
			returned.high = peek(address + 1);
			return returned.word;
		}

		uint8_t read() {
			auto content = reference();
			fireReadBusEvent();
			return content;
		}

		uint8_t read(uint16_t offset) {
			ADDRESS().word = offset;
			return read();
		}

		uint8_t read(register16_t address) {
			ADDRESS() = address;
			return read();
		}

		void write(uint8_t value) {
			reference() = value;
			fireWriteBusEvent();
		}

		void write(uint16_t offset, uint8_t value) {
			ADDRESS().word = offset;
			write(value);
		}

		void write(register16_t address, uint8_t value) {
			ADDRESS() = address;
			write(value);
		}

	protected:
		void fireReadBusEvent() {
			ReadByte.fire(AddressEventArgs(ADDRESS().word, DATA()));
		}

		void fireWriteBusEvent() {
			WrittenByte.fire(AddressEventArgs(ADDRESS().word, DATA()));
		}

		virtual uint8_t& reference(uint16_t address, bool& rom) = 0;

		uint8_t& reference() {
			bool rom;
			auto& value = reference(ADDRESS().word, rom);
			return rom ? placeDATA(value) : referenceDATA(value);
		}

	private:
		uint8_t m_temporary;	// Used to simulate ROM
		register16_t m_address;
		uint8_t* m_data;
	};
}