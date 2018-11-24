#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <vector>

#include "Chip.h"
#include "Signal.h"
#include "Register.h"
#include "EventArgs.h"
#include "MemoryMapping.h"

namespace EightBit {
	class Bus {
	public:
		virtual ~Bus() = default;

		Signal<EventArgs> WritingByte;
		Signal<EventArgs> WrittenByte;

		Signal<EventArgs> ReadingByte;
		Signal<EventArgs> ReadByte;

		auto ADDRESS() const { return m_address; }
		auto& ADDRESS() { return m_address; }

		auto DATA() const { return m_data; }
		auto& DATA() { return m_data; }

		auto peek() { return reference(); }
		auto peek(const uint16_t address) { return reference(address); }
		auto peek(const register16_t address) { return reference(address.word); }
		void poke(const uint8_t value) { reference() = value; }
		void poke(const uint16_t address, const uint8_t value) { reference(address) = value; }
		void poke(const register16_t address, const uint8_t value) { reference(address.word) = value; }

		uint8_t read();
		template<class T> auto read(const T address) {
			ADDRESS() = address;
			return read();
		}

		void write();
		void write(uint8_t value);
		template<class T> void write(const T offset, const uint8_t value) {
			ADDRESS() = offset;
			write(value);
		}

		virtual void powerOn();
		virtual void powerOff();

	protected:
		virtual void initialise() = 0;

		virtual MemoryMapping mapping(uint16_t address) = 0;
		uint8_t& reference(uint16_t address);
		auto& reference(const register16_t address) { return reference(address.word); }
		uint8_t& reference() { return reference(ADDRESS()); }

		static std::map<uint16_t, std::vector<uint8_t>> parseHexFile(std::string path);
		void loadHexFile(std::string path);

	private:
		uint8_t m_data = Chip::Mask8;
		register16_t m_address = Chip::Mask16;
	};
}
