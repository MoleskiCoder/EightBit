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
		static int8_t signExtend(int b, uint8_t x);

		Bus& BUS() { return m_bus; }

		register16_t& PC() { return m_pc; }

		PinLevel& RESET() { return m_resetLine; }
		PinLevel& HALT() { return m_haltLine; }
		PinLevel& INT() { return m_intLine; }
		PinLevel& IRQ() { return INT(); }	// Synonym

		virtual void powerOn();
		void reset() { lower(RESET()); }

		int run(int limit);
		virtual int step() = 0;
		virtual int execute(uint8_t opcode) = 0;

		int cycles() const { return m_cycles; }

		virtual register16_t peekWord(register16_t address) = 0;
		virtual void pokeWord(register16_t address, register16_t value) = 0;

	protected:
		Processor(Bus& memory);
		virtual ~Processor() = default;

		bool halted() { return lowered(HALT()); }
		void halt() { --PC();  lower(HALT()); }
		void proceed() { ++PC(); raise(HALT()); }

		virtual void handleRESET();
		virtual void handleINT();
		virtual void handleIRQ();

		uint8_t getBytePaged(const uint8_t page, const uint8_t offset) {
			return BUS().read(register16_t(offset, page));
		}

		void setBytePaged(const uint8_t page, const uint8_t offset, const uint8_t value) {
			BUS().write(register16_t(offset, page), value);
		}

		uint8_t fetchByte() {
			return BUS().read(PC()++);
		}

		virtual register16_t getWord() = 0;
		virtual void setWord(register16_t value) = 0;

		virtual register16_t getWordPaged(uint8_t page, uint8_t offset) = 0;
		virtual void setWordPaged(uint8_t page, uint8_t offset, register16_t value) = 0;

		virtual register16_t fetchWord() = 0;

		virtual void push(uint8_t value) = 0;
		virtual uint8_t pop() = 0;

		virtual void pushWord(const register16_t value) = 0;
		virtual register16_t popWord() = 0;

		register16_t getWord(const register16_t address) {
			BUS().ADDRESS() = address;
			return getWord();
		}

		void setWord(const register16_t address, const register16_t value) {
			BUS().ADDRESS() = address;
			return setWord(value);
		}

		void jump(const register16_t destination) {
			PC() = destination;
		}

		void call(const register16_t destination) {
			pushWord(PC());
			jump(destination);
		}

		virtual void ret() {
			jump(popWord());
		}

		void resetCycles() { m_cycles = 0; }
		void addCycles(const int extra) { m_cycles += extra; }
		void addCycle() { ++m_cycles;  }

	private:
		Bus& m_bus;
		int m_cycles = 0;
		register16_t m_pc;

		PinLevel m_intLine = Low;
		PinLevel m_haltLine = Low;
		PinLevel m_resetLine = Low;
	};
}
