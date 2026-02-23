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
		virtual ~Bus() noexcept = default;

		Signal<EventArgs> WritingByte;
		Signal<EventArgs> WrittenByte;

		Signal<EventArgs> ReadingByte;
		Signal<EventArgs> ReadByte;

		[[nodiscard]] constexpr auto ADDRESS() const noexcept { return m_address; }
		[[nodiscard]] constexpr auto& ADDRESS() noexcept { return m_address; }

		[[nodiscard]] constexpr auto DATA() const noexcept { return m_data; }
		[[nodiscard]] constexpr auto& DATA() noexcept { return m_data; }

		[[nodiscard]] virtual uint8_t peek(const uint16_t address) noexcept { return reference(address); }
		[[nodiscard]] auto peek(const register16_t address) noexcept { return peek(address.word); }

		void poke(const uint8_t value) noexcept { reference() = value; }
		virtual void poke(const uint16_t address, const uint8_t value) noexcept { reference(address) = value; }
		void poke(const register16_t address, const uint8_t value) noexcept { poke(address.word, value); }

		[[nodiscard]] uint8_t read();
		void write();

		virtual void raisePOWER() noexcept;
		virtual void lowerPOWER() noexcept;

		virtual void initialise() = 0;

	protected:
		[[nodiscard]] uint8_t& reference(uint16_t address) noexcept;
		[[nodiscard]] uint8_t& reference() noexcept { return reference(ADDRESS().word); }

		void loadHexFile(const std::string& path);

	private:
		uint8_t m_data = Chip::Mask8;
		register16_t m_address = Chip::Mask16;
	};
}
