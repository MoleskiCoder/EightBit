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

		static bool raised(PinLevel line) { return line == High; }
		static void raise(PinLevel& line) { line = High; }
		static bool lowered(PinLevel line) { return line == Low; }
		static void lower(PinLevel& line) { line = Low; }

		static int highNibble(int value) { return value >> 4; }
		static int lowNibble(int value) { return value & Mask4; }

		static int promoteNibble(int value) { return value << 4; }
		static int demoteNibble(int value) { return highNibble(value); }

		Bus& BUS() { return m_bus; }

		register16_t& PC() { return m_pc; }

		PinLevel& RESET() { return m_resetLine; }	// In
		PinLevel& HALT() { return m_haltLine; }		// Out
		PinLevel& INT() { return m_intLine; }		// In
		PinLevel& NMI() { return m_nmiLine; }		// In
		PinLevel& POWER() { return m_powerLine; }	// In

		bool powered() { return raised(POWER()); }
		virtual void powerOn() { raise(POWER()); raise(HALT()); reset(); }
		void powerOff() { lower(POWER()); }

		int run(int limit);
		virtual int singleStep();
		virtual int step() = 0;

		virtual int execute(uint8_t opcode) = 0;

	protected:
		static void clearFlag(uint8_t& f, int flag) { f &= ~flag; }
		static void setFlag(uint8_t& f, int flag) { f |= flag; }

		static void setFlag(uint8_t& f, int flag, int condition) { setFlag(f, flag, condition != 0); }
		static void setFlag(uint8_t& f, int flag, uint32_t condition) { setFlag(f, flag, condition != 0); }
		static void setFlag(uint8_t& f, int flag, bool condition) { condition ? setFlag(f, flag) : clearFlag(f, flag); }

		static void clearFlag(uint8_t& f, int flag, int condition) { clearFlag(f, flag, condition != 0); }
		static void clearFlag(uint8_t& f, int flag, uint32_t condition) { clearFlag(f, flag, condition != 0); }
		static void clearFlag(uint8_t& f, int flag, bool condition) { condition ? clearFlag(f, flag) : setFlag(f, flag); }

		Processor(Bus& memory);
		virtual ~Processor() = default;

		virtual void reset();

		bool halted() { return lowered(HALT()); }
		void halt() { --PC().word;  lower(HALT()); }
		void proceed() { ++PC().word; raise(HALT()); }

		uint8_t fetchByte() {
			return BUS().read(PC().word++);
		}

		register16_t fetchWord() {
			register16_t returned;
			returned.low = fetchByte();
			returned.high = fetchByte();
			return returned;
		}

		virtual void push(uint8_t value) = 0;
		virtual uint8_t pop() = 0;

		void pushWord(register16_t value) {
			push(value.high);
			push(value.low);
		}

		register16_t popWord() {
			register16_t returned;
			returned.low = pop();
			returned.high = pop();
			return returned;
		}

		void jump(register16_t destination) {
			PC() = destination;
		}

		void call(register16_t destination) {
			pushWord(PC());
			jump(destination);
		}

		void ret() {
			jump(popWord());
		}

		int cycles() const { return m_cycles; }
		void resetCycles() { m_cycles = 0; }
		void addCycles(int extra) { m_cycles += extra; }
		void addCycle() { ++m_cycles;  }

	private:
		Bus& m_bus;
		int m_cycles = 0;
		register16_t m_pc = { { 0, 0 } };

		PinLevel m_intLine = Low;
		PinLevel m_nmiLine = Low;
		PinLevel m_haltLine = Low;
		PinLevel m_resetLine = Low;
		PinLevel m_powerLine = Low;
	};
}
