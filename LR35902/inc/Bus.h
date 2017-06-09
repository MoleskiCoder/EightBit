#pragma once

#include "Memory.h"

namespace EightBit {
	class Bus : public Memory {
	public:

		enum {
			TotalLineCount = 154
		};

		enum {
			VideoRam = 0x8000
		};

		enum {

			BASE = 0xFF00,

			// Port/Mode Registers
			P1 = 0x0,
			SB = 0x1,
			SC = 0x2,
			DIV = 0x4,
			TIMA = 0x5,
			TMA = 0x6,
			TAC = 0x7,

			// Interrupt Flags
			IF = 0xF,
			IE = 0xFF,

			// LCD Display Registers
			LCDC = 0x40,
			STAT = 0x41,
			SCY = 0x42,
			SCX = 0x43,
			LY = 0x44,
			LYC = 0x45,
			DMA = 0x46,
			BGP = 0x47,
			OBP0 = 0x48,
			OBP1 = 0x49,
			WY = 0x4A,
			WX = 0x4B,

			// Sound Registers
			NR10 = 0x10,
			NR11 = 0x11,
			NR12 = 0x12,
			NR13 = 0x13,
			NR14 = 0x14,
			NR21 = 0x16,
			NR22 = 0x17,
			NR23 = 0x18,
			NR24 = 0x19,
			NR30 = 0x1A,
			NR31 = 0x1B,
			NR32 = 0x1C,
			NR33 = 0x1D,
			NR34 = 0x1E,
			NR41 = 0x20,
			NR42 = 0x21,
			NR43 = 0x22,
			NR44 = 0x23,
			NR50 = 0x24,
			NR51 = 0x25,
			NR52 = 0x26,

			WPRAM_START = 0x30,
			WPRAM_END = 0x3F,

			// Boot rom control
			BOOT_DISABLE = 0x50,
		};

		Bus();

		void reset();

		uint8_t& REG(int offset) {
			ADDRESS().word = BASE + offset;
			return Memory::reference();
		}

		void incrementLY() {
			REG(LY) = (REG(LY) + 1) % TotalLineCount;
		}

		void resetLY() {
			REG(LY) = 0;
		}

		void loadBootRom(const std::string& path);

		bool isBootRom(uint16_t address) const {
			return (address < m_boot.size()) && (peek(BASE + BOOT_DISABLE) == 0);
		}

		virtual uint8_t peek(uint16_t address) const;

		virtual uint8_t& reference();

	private:
		std::array<uint8_t, 0x100> m_boot;
	};
}