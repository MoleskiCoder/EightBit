#pragma once

#include <cstdint>

#include "Memory.h"

namespace EightBit {
	class Processor {
	public:
		enum Masks {
			Mask1 = 0x01,
			Mask2 = 0x03,
			Mask3 = 0x07,
			Mask4 = 0x0f,
			Mask5 = 0x1f,
			Mask6 = 0x3f,
			Mask7 = 0x7f,
			Mask8 = 0xff,
		};

		enum Bits {
			Bit16 = 0x10000,
			Bit15 = 0x8000,
			Bit14 = 0x4000,
			Bit13 = 0x2000,
			Bit12 = 0x1000,
			Bit11 = 0x800,
			Bit10 = 0x400,
			Bit9 = 0x200,
			Bit8 = 0x100,
			Bit7 = 0x80,
			Bit6 = 0x40,
			Bit5 = 0x20,
			Bit4 = 0x10,
			Bit3 = 0x8,
			Bit2 = 0x4,
			Bit1 = 0x2,
			Bit0 = 0x1,
		};

		static int highNibble(int value) { return value >> 4; }
		static int lowNibble(int value) { return value & Mask4; }

		static int promoteNibble(int value) { return value << 4; }
		static int demoteNibble(int value) { return highNibble(value); }

		const Memory& getMemory() const { return m_memory; }

		register16_t& PC() { return pc; }

		bool isHalted() const { return m_halted; }
		void halt() { --PC().word;  m_halted = true; }

		virtual void initialise();

		void reset();

	protected:
		static void clearFlag(uint8_t& f, int flag) { f &= ~flag; }
		static void setFlag(uint8_t& f, int flag) { f |= flag; }

		static void setFlag(uint8_t& f, int flag, int condition) { setFlag(f, flag, condition != 0); }
		static void setFlag(uint8_t& f, int flag, uint32_t condition) { setFlag(f, flag, condition != 0); }
		static void setFlag(uint8_t& f, int flag, bool condition) { condition ? setFlag(f, flag) : clearFlag(f, flag); }

		static void clearFlag(uint8_t& f, int flag, int condition) { clearFlag(f, flag, condition != 0); }
		static void clearFlag(uint8_t& f, int flag, uint32_t condition) { clearFlag(f, flag, condition != 0); }
		static void clearFlag(uint8_t& f, int flag, bool condition) { condition ? clearFlag(f, flag) : setFlag(f, flag); }

		Processor(Memory& memory);

		Memory& m_memory;
		int cycles;

	private:
		register16_t pc;
		bool m_halted;
	};
}