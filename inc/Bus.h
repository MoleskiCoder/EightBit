#pragma once

#include <cstdint>
#include <string>

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
		[[nodiscard]] virtual uint8_t peek(const uint16_t address) { return reference(address); }
		[[nodiscard]] auto peek(const register16_t address) { return peek(address.word); }
		void poke(const uint8_t value) { reference() = value; }
		virtual void poke(const uint16_t address, const uint8_t value) { reference(address) = value; }
		void poke(const register16_t address, const uint8_t value) { poke(address.word, value); }

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

		virtual void raisePOWER();
		virtual void lowerPOWER();

		virtual void initialise() = 0;

	protected:
		[[nodiscard]] uint8_t& reference(uint16_t address);
		[[nodiscard]] auto& reference(const register16_t address) { return reference(address.word); }
		[[nodiscard]] uint8_t& reference() { return reference(ADDRESS()); }

		void loadHexFile(std::string path);

	private:
		uint8_t m_data = Chip::Mask8;
		register16_t m_address = Chip::Mask16;
	};
}
