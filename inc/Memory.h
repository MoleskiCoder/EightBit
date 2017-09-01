#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "Signal.h"
#include "AddressEventArgs.h"

#if defined(_M_X64) || defined(_M_IX86 )
#	define HOST_LITTLE_ENDIAN
#else
#	define HOST_BIG_ENDIAN
#endif

namespace EightBit {

	typedef union {
		struct {
#ifdef HOST_LITTLE_ENDIAN
			uint8_t low;
			uint8_t high;
#endif
#ifdef HOST_BIG_ENDIAN
			uint8_t high;
			uint8_t low;
#endif
		};
		uint16_t word;
	} register16_t;

	class Memory {
	public:
		Memory(uint16_t addressMask);

		// Only fired with read/write methods
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

		uint8_t peek(uint16_t address);
		void poke(uint16_t address, uint8_t value);

		uint16_t peekWord(uint16_t address);

		virtual int effectiveAddress(int address) const {
			return address & m_addressMask;
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

		virtual void clear();
		void loadRom(const std::string& path, uint16_t offset);
		void loadRam(const std::string& path, uint16_t offset);

		void lock(int address, int size) {
			std::fill(m_locked.begin() + address, m_locked.begin() + address + size, true);
		}

	protected:
		std::vector<uint8_t> m_bus;
		std::vector<bool> m_locked;

		uint16_t m_addressMask;		// Mirror
		uint8_t m_temporary;	// Used to simulate ROM
		register16_t m_address;
		uint8_t* m_data;

		static int loadBinary(const std::string& path, std::vector<uint8_t>& output, int offset = 0, int maximumSize = -1);

		void fireReadBusEvent() {
			ReadByte.fire(AddressEventArgs(ADDRESS().word, DATA()));
		}

		void fireWriteBusEvent() {
			WrittenByte.fire(AddressEventArgs(ADDRESS().word, DATA()));
		}

		virtual uint8_t& reference(uint16_t address, bool& rom) {
			rom = m_locked[address];
			return m_bus[address];
		}

		uint8_t& reference() {
			bool rom;
			auto& value = reference(ADDRESS().word, rom);
			return rom ? placeDATA(value) : referenceDATA(value);
		}

		int loadMemory(const std::string& path, uint16_t offset);
	};
}