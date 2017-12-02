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

		static int highNibble(int value) { return value >> 4; }
		static int lowNibble(int value) { return value & Mask4; }

		static int promoteNibble(int value) { return value << 4; }
		static int demoteNibble(int value) { return highNibble(value); }

		Bus& BUS() { return m_bus; }

		register16_t& PC() { return m_pc; }
		register16_t& MEMPTR() { return m_memptr; }

		bool halted() const { return m_halted; }
		void halt() { --PC().word;  m_halted = true; }
		void proceed() { ++PC().word; m_halted = false; }

		bool powered() const { return m_power; }
		void powerOn() { m_power = true; }
		void powerOff() { m_power = false; }

		virtual void initialise();
		virtual void reset();

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

		virtual uint8_t fetchByte();
		virtual void fetchWord(register16_t& output);

		void fetchWord() {
			fetchWord(MEMPTR());
		}

		uint8_t getByte() { return BUS().read(); }
		template<class T> uint8_t getByte(T offset) { return BUS().read(offset); }

		void setByte(uint8_t value) { BUS().write(value); }
		template<class T> void setByte(T offset, uint8_t value) { BUS().write(offset, value); }

		virtual void push(uint8_t value) = 0;
		virtual uint8_t pop() = 0;

		void pushWord(register16_t value) {
			push(value.high);
			push(value.low);
		}

		void popWord(register16_t& output) {
			output.low = pop();
			output.high = pop();
		}

		void jump() {
			PC() = MEMPTR();
		}

		void call() {
			pushWord(PC());
			jump();
		}

		void ret() {
			popWord(MEMPTR());
			jump();
		}

		int cycles() const { return m_cycles; }
		void resetCycles() { m_cycles = 0; }
		void addCycles(int extra) { m_cycles += extra; }
		void addCycle() { ++m_cycles;  }

	private:
		Bus& m_bus;
		int m_cycles = 0;
		register16_t m_pc = { { 0, 0 } };
		register16_t m_memptr = { { 0, 0 } };
		bool m_halted = false;
		bool m_power = false;
	};
}
