#pragma once

#include <cstdint>

#include "ClockedChip.h"
#include "Bus.h"
#include "Register.h"

#include "EightBitCompilerDefinitions.h"

namespace EightBit {
	class Processor : public ClockedChip {
	public:
		// http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
		// b: number of bits representing the number in x
		// x: sign extend this b-bit number to r
		[[nodiscard]] static constexpr int8_t signExtend(int b, uint8_t x) noexcept {
			const uint8_t m = bit(b - 1); // mask can be pre-computed if b is fixed
			x = x & (bit(b) - 1);  // (Skip this if bits in x above position b are already zero.)
			const auto result = (x ^ m) - m;
			return result;
		}

		Processor(const Processor& rhs) noexcept;
		bool operator==(const Processor& rhs) const noexcept;

		[[nodiscard]] constexpr auto& PC() noexcept { return m_pc; }
		[[nodiscard]] constexpr const auto& PC() const noexcept { return m_pc; }

		int run(int limit) noexcept;
		virtual int step() noexcept = 0;
		virtual int execute() noexcept = 0;
		int execute(uint8_t value) noexcept;

		[[nodiscard]] virtual register16_t peekWord(register16_t address) noexcept = 0;
		virtual void pokeWord(register16_t address, register16_t value) noexcept = 0;

		DECLARE_PIN_INPUT(RESET)
		DECLARE_PIN_INPUT(INT)

	protected:
		Processor(Bus& memory) noexcept;

		[[nodiscard]] constexpr auto& opcode() noexcept { return m_opcode; }
		[[nodiscard]] constexpr auto& BUS() noexcept { return m_bus; }

		virtual void handleRESET();
		virtual void handleINT();

		void memoryWrite(register16_t address, uint8_t data);
		void memoryWrite(register16_t address);
		void memoryWrite(uint8_t data);
		virtual void memoryWrite();
		virtual void busWrite();

		uint8_t memoryRead(register16_t address);
		virtual uint8_t memoryRead();
		virtual uint8_t busRead();

		uint8_t getBytePaged() { return memoryRead(); }
		uint8_t getBytePaged(uint8_t page, uint8_t offset);
		void setBytePaged(uint8_t value) { memoryWrite(value); }
		void setBytePaged(uint8_t page, uint8_t offset, uint8_t value);

		uint8_t fetchByte();

		[[nodiscard]] virtual register16_t getWord() = 0;
		virtual void setWord(register16_t value) = 0;

		[[nodiscard]] register16_t getWordPaged(uint8_t page, uint8_t offset);
		[[nodiscard]] virtual register16_t getWordPaged() = 0;
		void setWordPaged(uint8_t page, uint8_t offset, register16_t value);
		virtual void setWordPaged(register16_t value) = 0;

		[[nodiscard]] virtual register16_t fetchWord() = 0;

		virtual void push(uint8_t value) = 0;
		[[nodiscard]] virtual uint8_t pop() = 0;

		virtual void pushWord(register16_t value) = 0;
		[[nodiscard]] virtual register16_t popWord() = 0;

		[[nodiscard]] register16_t getWord(register16_t address);
		void setWord(register16_t address, register16_t value);

		void jump(const register16_t destination) noexcept;
		virtual void call(register16_t destination);
		virtual void ret();

	private:
		Bus& m_bus;
		uint8_t m_opcode = Mask8;
		register16_t m_pc;
	};
}
