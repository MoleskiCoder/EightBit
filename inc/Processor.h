#pragma once

#include <cstdint>

#include "Bus.h"
#include "Register.h"

#include "EightBitCompilerDefinitions.h"

namespace EightBit {
	class Processor {
	public:
		enum Bits {
			Bit0 = 1,
			Bit1 = Bit0 << 1,
			Bit2 = Bit1 << 1,
			Bit3 = Bit2 << 1,
			Bit4 = Bit3 << 1,
			Bit5 = Bit4 << 1,
			Bit6 = Bit5 << 1,
			Bit7 = Bit6 << 1,
			Bit8 = Bit7 << 1,
			Bit9 = Bit8 << 1,
			Bit10 = Bit9 << 1,
			Bit11 = Bit10 << 1,
			Bit12 = Bit11 << 1,
			Bit13 = Bit12 << 1,
			Bit14 = Bit13 << 1,
			Bit15 = Bit14 << 1,
			Bit16 = Bit15 << 1
		};

		enum Masks {
			Mask1 = Bit1 - 1,
			Mask2 = Bit2 - 1,
			Mask3 = Bit3 - 1,
			Mask4 = Bit4 - 1,
			Mask5 = Bit5 - 1,
			Mask6 = Bit6 - 1,
			Mask7 = Bit7 - 1,
			Mask8 = Bit8 - 1,
			Mask9 = Bit9 - 1,
			Mask10 = Bit10 - 1,
			Mask11 = Bit11 - 1,
			Mask12 = Bit12 - 1,
			Mask13 = Bit13 - 1,
			Mask14 = Bit14 - 1,
			Mask15 = Bit15 - 1,
			Mask16 = Bit16 - 1
		};

		enum PinLevel {
			Low, High
		};

		static bool raised(const PinLevel line) { return line == High; }
		static void raise(PinLevel& line) { line = High; }
		static bool lowered(const PinLevel line) { return line == Low; }
		static void lower(PinLevel& line) { line = Low; }

		static int highNibble(const int value) { return value >> 4; }
		static int lowNibble(const int value) { return value & Mask4; }

		static int higherNibble(const int value) { return value & 0xf0; }
		static int lowerNibble(const int value) { return lowNibble(value); }

		static int promoteNibble(const int value) { return value << 4; }
		static int demoteNibble(const int value) { return highNibble(value); }

		Bus& BUS() { return m_bus; }

		register16_t& PC() { return m_pc; }

		PinLevel& RESET() { return m_resetLine; }
		PinLevel& HALT() { return m_haltLine; }
		PinLevel& INT() { return m_intLine; }
		PinLevel& NMI() { return m_nmiLine; }
		PinLevel& POWER() { return m_powerLine; }

		bool powered() { return raised(POWER()); }
		virtual void powerOn() { raise(POWER()); raise(HALT()); reset(); }
		void powerOff() { lower(POWER()); }

		int run(int limit);
		virtual int singleStep();
		virtual int step() = 0;

		virtual int execute(uint8_t opcode) = 0;

	protected:
		static void clearFlag(uint8_t& f, const int flag) { f &= ~flag; }
		static void setFlag(uint8_t& f, const int flag) { f |= flag; }

		static void setFlag(uint8_t& f, const int flag, const int condition) { setFlag(f, flag, !!condition); }
		static void setFlag(uint8_t& f, const int flag, const uint32_t condition) { setFlag(f, flag, !!condition); }
		static void setFlag(uint8_t& f, const int flag, const bool condition) { condition ? setFlag(f, flag) : clearFlag(f, flag); }

		static void clearFlag(uint8_t& f, const int flag, const int condition) { clearFlag(f, flag, !!condition); }
		static void clearFlag(uint8_t& f, const int flag, const uint32_t condition) { clearFlag(f, flag, !!condition); }
		static void clearFlag(uint8_t& f, const int flag, const bool condition) { condition ? clearFlag(f, flag) : setFlag(f, flag); }

		Processor(Bus& memory);
		virtual ~Processor() = default;

		virtual void reset();

		bool halted() { return lowered(HALT()); }
		void halt() { --PC();  lower(HALT()); }
		void proceed() { ++PC(); raise(HALT()); }

		uint8_t fetchByte() {
			return BUS().read(PC()++);
		}

		register16_t fetchWord() {
			const auto low = fetchByte();
			const auto high = fetchByte();
			return register16_t(low, high);
		}

		virtual void push(uint8_t value) = 0;
		virtual uint8_t pop() = 0;

		void pushWord(const register16_t value) {
			push(value.high);
			push(value.low);
		}

		register16_t popWord() {
			const auto low = pop();
			const auto high = pop();
			return register16_t(low, high);
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

		int cycles() const { return m_cycles; }
		void resetCycles() { m_cycles = 0; }
		void addCycles(const int extra) { m_cycles += extra; }
		void addCycle() { ++m_cycles;  }

	private:
		Bus& m_bus;
		int m_cycles = 0;
		register16_t m_pc;

		PinLevel m_intLine = Low;
		PinLevel m_nmiLine = Low;
		PinLevel m_haltLine = Low;
		PinLevel m_resetLine = Low;
		PinLevel m_powerLine = Low;
	};
}
