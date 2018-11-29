#pragma once

#include <cstdint>

#include "Chip.h"
#include "Bus.h"
#include "Register.h"

#include "EightBitCompilerDefinitions.h"

namespace EightBit {
	class Processor : public Chip {
	public:
		// b: number of bits representing the number in x
		// x: sign extend this b-bit number to r
		[[nodiscard]] static int8_t signExtend(int b, uint8_t x);

		~Processor() {};

		[[nodiscard]] auto& PC() noexcept { return m_pc; }

		[[nodiscard]] auto& RESET() noexcept { return m_resetLine; }
		[[nodiscard]] auto& HALT() noexcept { return m_haltLine; }
		[[nodiscard]] auto& INT() noexcept { return m_intLine; }
		[[nodiscard]] auto& IRQ() noexcept { return INT(); }	// Synonym

		void powerOn() override;
		void reset() noexcept { lower(RESET()); }

		int run(int limit);
		virtual int step() = 0;
		virtual int execute(uint8_t opcode) = 0;

		[[nodiscard]] auto cycles() const noexcept { return m_cycles; }

		[[nodiscard]] virtual register16_t peekWord(register16_t address) = 0;
		virtual void pokeWord(register16_t address, register16_t value) = 0;

	protected:
		Processor(Bus& memory);

		[[nodiscard]] auto& BUS() noexcept { return m_bus; }

		[[nodiscard]] auto halted() noexcept { return lowered(HALT()); }
		void halt() noexcept { --PC();  lower(HALT()); }
		void proceed() noexcept { ++PC(); raise(HALT()); }

		virtual void handleRESET();
		virtual void handleINT();
		virtual void handleIRQ();

		[[nodiscard]] auto getBytePaged(const uint8_t page, const uint8_t offset) {
			return BUS().read(register16_t(offset, page));
		}

		void setBytePaged(const uint8_t page, const uint8_t offset, const uint8_t value) {
			BUS().write(register16_t(offset, page), value);
		}

		[[nodiscard]] auto fetchByte() {
			return BUS().read(PC()++);
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
			return setWord(value);
		}

		void jump(const register16_t destination) noexcept {
			PC() = destination;
		}

		void call(const register16_t destination) {
			pushWord(PC());
			jump(destination);
		}

		virtual void ret();

		void resetCycles() noexcept { m_cycles = 0; }
		void addCycles(const int extra) noexcept { m_cycles += extra; }
		void addCycle() noexcept { ++m_cycles;  }

	private:
		Bus& m_bus;
		int m_cycles = 0;
		register16_t m_pc;

		PinLevel m_intLine = Low;
		PinLevel m_haltLine = Low;
		PinLevel m_resetLine = Low;
	};
}
