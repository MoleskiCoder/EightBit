#pragma once

#include "Memory.h"
#include "Processor.h"

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

		// IF and IE flags
		enum Interrupts {
			VerticalBlank = Processor::Bit0,			// VBLANK
			DisplayControlStatus = Processor::Bit1,		// LCDC Status
			TimerOverflow = Processor::Bit2,			// Timer Overflow
			SerialTransfer = Processor::Bit3,			// Serial Transfer
			Keypad = Processor::Bit3					// Hi-Lo of P10-P13
		};

		Bus();

		void reset();

		virtual void clear() override;

		void triggerInterrupt(int cause) {
			writeRegister(IF, readRegister(IF) | cause);
		}

		void writeRegister(int offset, uint8_t content) {
			return Memory::write(BASE + offset, content);
		}

		uint8_t readRegister(int offset) {
			return Memory::read(BASE + offset);
		}

		void checkTimer(int cycles) {
			if (timerEnabled()) {
				m_timerCounter -= cycles;
				if (m_timerCounter <= 0) {
					m_timerCounter = m_timerRate;
					incrementTIMA();
				}
			}
		}

		int timerClockTicks() {
			switch (timerClock()) {
			case 0b00:
				return 1024;	// 4.096 Khz
			case 0b01:
				return 16;		// 262.144 Khz
			case 0b10:
				return 64;		// 65.536 Khz
			case 0b11:
				return 256;		// 16.384 Khz
			default:
				__assume(0);
			}
			throw std::domain_error("Invalid timer clock specification");
		}

		int timerClock() {
			return readRegister(TAC) & Processor::Mask2;
		}

		bool timerEnabled() {
			return !timerDisabled();
		}

		bool timerDisabled() {
			return (readRegister(TAC) & Processor::Bit2) == 0;
		}

		void incrementTIMA() {
			uint16_t updated = readRegister(TIMA) + 1;
			if (updated & Processor::Bit8) {
				triggerInterrupt(TimerOverflow);
				updated = readRegister(TMA);
			}
			writeRegister(TIMA, updated & Processor::Mask8);
		}

		void incrementLY() {
			writeRegister(LY, (readRegister(LY) + 1) % TotalLineCount);
		}

		void resetLY() {
			writeRegister(LY, 0);
		}

		bool bootRomDisabled() const { return m_disableBootRom; }
		bool bootRomEnabled() const { return !bootRomDisabled(); }

		void loadBootRom(const std::string& path);

		bool isBootRom(uint16_t address) const {
			return (address < m_boot.size()) && bootRomEnabled();
		}

		virtual uint8_t peek(uint16_t address) const;

		virtual uint8_t& reference();

	private:
		std::array<uint8_t, 0x100> m_boot;
		bool m_disableBootRom;

		int m_timerCounter;
		int m_timerRate;

		void Bus_WrittenByte(const AddressEventArgs& e);
	};
}