#pragma once

#include <array>
#include <cstdint>
#include <string>

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

		virtual register16_t& ADDRESS() { return m_address; }
		virtual uint8_t& DATA() { return *m_data; }

		virtual uint8_t& placeDATA(uint8_t value) {
			m_temporary = value;
			m_data = &m_temporary;
			return DATA();
		}

		virtual uint8_t& referenceDATA(uint8_t& value) {
			m_data = &value;
			return DATA();
		}

		virtual uint8_t peek(uint16_t address) const;
		virtual uint16_t peekWord(uint16_t address) const;

		virtual uint8_t get(uint16_t address) {
			ADDRESS().word = address;
			return reference();
		}

		virtual register16_t getWord(uint16_t address);

		virtual void set(uint16_t address, uint8_t value) {
			ADDRESS().word = address;
			reference() = value;
		}

		virtual void setWord(uint16_t address, register16_t value);

		virtual uint8_t& reference() {
			uint16_t effective = ADDRESS().word & m_addressMask;
			return m_locked[effective] ? placeDATA(m_bus[effective]) : referenceDATA(m_bus[effective]);
		}

		void clear();
		void loadRom(const std::string& path, uint16_t offset);
		void loadRam(const std::string& path, uint16_t offset);

	protected:
		std::array<uint8_t, 0x10000> m_bus;
		std::array<bool, 0x10000> m_locked;

		uint16_t m_addressMask;		// Mirror
		uint8_t m_temporary;	// Used to simulate ROM
		register16_t m_address;
		uint8_t* m_data;

		int loadMemory(const std::string& path, uint16_t offset);
	};
}