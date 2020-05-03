#pragma once

#include <cstdint>

#include "ClockedChip.h"
#include "Bus.h"
#include "Register.h"

#include "EightBitCompilerDefinitions.h"

namespace EightBit {
	class Processor : public ClockedChip {
	public:
		// b: number of bits representing the number in x
		// x: sign extend this b-bit number to r
		[[nodiscard]] static int8_t signExtend(int b, uint8_t x) noexcept;

		~Processor() = default;

		[[nodiscard]] auto& PC() noexcept { return m_pc; }

		int run(int limit);
		virtual int step() = 0;
		virtual int execute() = 0;
		int execute(uint8_t value);

		[[nodiscard]] virtual register16_t peekWord(register16_t address) = 0;
		virtual void pokeWord(register16_t address, register16_t value) = 0;

		DECLARE_PIN_INPUT(RESET)
		DECLARE_PIN_INPUT(INT)

	protected:
		Processor(Bus& memory);

		[[nodiscard]] auto& opcode() noexcept { return m_opcode; }
		[[nodiscard]] auto& BUS() noexcept { return m_bus; }

		virtual void handleRESET();
		virtual void handleINT();

		void memoryWrite(register16_t address, uint8_t data);
		void memoryWrite(uint8_t data);
		virtual void memoryWrite();
		virtual void busWrite();

		uint8_t memoryRead(register16_t address);
		virtual uint8_t memoryRead();
		virtual uint8_t busRead();

		auto getBytePaged(const uint8_t page, const uint8_t offset) {
			return memoryRead(register16_t(offset, page));
		}

		void setBytePaged(const uint8_t page, const uint8_t offset, const uint8_t value) {
			memoryWrite(register16_t(offset, page), value);
		}

		auto fetchByte() {
			return memoryRead(PC()++);
		}

		[[nodiscard]] virtual register16_t getWord() = 0;
		virtual void setWord(register16_t value) = 0;

		[[nodiscard]] virtual register16_t getWordPaged(uint8_t page, uint8_t offset) = 0;
		virtual void setWordPaged(uint8_t page, uint8_t offset, register16_t value) = 0;

		[[nodiscard]] virtual register16_t fetchWord() = 0;

		virtual void push(uint8_t value) = 0;
		[[nodiscard]] virtual uint8_t pop() = 0;

		virtual void pushWord(register16_t value) = 0;
		[[nodiscard]] virtual register16_t popWord() = 0;

		[[nodiscard]] auto getWord(const register16_t address) {
			BUS().ADDRESS() = address;
			return getWord();
		}

		void setWord(const register16_t address, const register16_t value) {
			BUS().ADDRESS() = address;
			setWord(value);
		}

		void jump(const register16_t destination) noexcept {
			PC() = destination;
		}

		virtual void call(const register16_t destination) {
			pushWord(PC());
			jump(destination);
		}

		virtual void ret();

	private:
		Bus& m_bus;
		uint8_t m_opcode = Mask8;
		register16_t m_pc;
	};
}
