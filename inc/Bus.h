#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <vector>

#include "Chip.h"
#include "Signal.h"
#include "Register.h"
#include "EventArgs.h"
#include "Mapper.h"

namespace EightBit {
	class Bus : public Mapper {
	public:
		virtual ~Bus() = default;

		Signal<EventArgs> WritingByte;
		Signal<EventArgs> WrittenByte;

		Signal<EventArgs> ReadingByte;
		Signal<EventArgs> ReadByte;

		[[nodiscard]] auto ADDRESS() const noexcept { return m_address; }
		[[nodiscard]] auto& ADDRESS() noexcept { return m_address; }

		[[nodiscard]] auto DATA() const noexcept { return m_data; }
		[[nodiscard]] auto& DATA() noexcept { return m_data; }

		[[nodiscard]] auto peek() { return reference(); }
		[[nodiscard]] auto peek(const uint16_t address) { return reference(address); }
		[[nodiscard]] auto peek(const register16_t address) { return reference(address.word); }
		void poke(const uint8_t value) { reference() = value; }
		void poke(const uint16_t address, const uint8_t value) { reference(address) = value; }
		void poke(const register16_t address, const uint8_t value) { reference(address.word) = value; }

		[[nodiscard]] uint8_t read();
		template<class T> [[nodiscard]] auto read(const T address) {
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

		[[nodiscard]] uint8_t& reference(uint16_t address);
		[[nodiscard]] auto& reference(const register16_t address) { return reference(address.word); }
		[[nodiscard]] uint8_t& reference() { return reference(ADDRESS()); }

		[[nodiscard]] static std::map<uint16_t, std::vector<uint8_t>> parseHexFile(std::string path);
		void loadHexFile(std::string path);

	private:
		uint8_t m_data = Chip::Mask8;
		register16_t m_address = Chip::Mask16;
	};
}
