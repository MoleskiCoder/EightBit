#pragma once

#include <cstdint>

#include <Ram.h>
#include <Processor.h>
#include <Register.h>
#include <Signal.h>

namespace EightBit {
	namespace GameBoy {

		class Bus;

		class IoRegisters final : public EightBit::Ram {
		public:

			enum {

				BASE = 0xFF00,

				// Port/Mode Registers
				P1 = 0x0,		// R/W	Mask5
				SB = 0x1,		// R/W	Mask8
				SC = 0x2,		// R/W	Bit7 | Bit0

				// Timer control
				DIV = 0x4,		// R/W	Mask8
				TIMA = 0x5,		// R/W	Mask8
				TMA = 0x6,		// R/W	Mask8
				TAC = 0x7,		// R/W	Mask3

				// Interrupt Flags
				IF = 0xF,		// R/W	Mask5
				IE = 0xFF,		// R/W	Mask5

				// Sound Registers
				NR10 = 0x10,	// R/W	Mask7
				NR11 = 0x11,	// R/W	Bit7 | Bit6
				NR12 = 0x12,	// R/W	Mask8
				NR13 = 0x13,	// W	0
				NR14 = 0x14,	// R/W	Bit6
				NR21 = 0x16,	// R/W	Bit7 | Bit6
				NR22 = 0x17,	// R/W	Mask8
				NR23 = 0x18,	// W	0
				NR24 = 0x19,	// R/W	Bit6
				NR30 = 0x1A,	// R/W	Bit7
				NR31 = 0x1B,	// R/W	Mask8
				NR32 = 0x1C,	// R/W	Bit6 | Bit5
				NR33 = 0x1D,	// W	0
				NR34 = 0x1E,	// R/W	Bit6
				NR41 = 0x20,	// R/W	Mask6
				NR42 = 0x21,	// R/W	Mask8
				NR43 = 0x22,	// R/W	Mask8
				NR44 = 0x23,	// R/W	Bit6
				NR50 = 0x24,	// R/W	Mask8
				NR51 = 0x25,	// R/W	Mask8
				NR52 = 0x26,	// R/W	Mask8	Mask8

				WAVE_PATTERN_RAM_START = 0x30,
				WAVE_PATTERN_RAM_END = 0x3F,

				// LCD Display Registers
				LCDC = 0x40,	// R/W	Mask8
				STAT = 0x41,	// R/W	Mask7
				SCY = 0x42,		// R/W	Mask8
				SCX = 0x43,		// R/W	Mask8
				LY = 0x44,		// R	Mask8	zeroed
				LYC = 0x45,		// R/W	Mask8
				DMA = 0x46,		// W	0
				BGP = 0x47,		// R/W	Mask8
				OBP0 = 0x48,	// R/W	Mask8
				OBP1 = 0x49,	// R/W	Mask8
				WY = 0x4A,		// R/W	Mask8
				WX = 0x4B,		// R/W	Mask8

				// Boot rom control
				BOOT_DISABLE = 0x50,
			};

			// IF and IE flags
			enum Interrupts {
				VerticalBlank = Processor::Bit0,			// VBLANK
				DisplayControlStatus = Processor::Bit1,		// LCDC Status
				TimerOverflow = Processor::Bit2,			// Timer Overflow
				SerialTransfer = Processor::Bit3,			// Serial Transfer
				KeypadPressed = Processor::Bit4				// Hi-Lo transition of P10-P13
			};

			enum LcdcControl {
				DisplayBackground = Processor::Bit0,
				ObjectEnable = Processor::Bit1,
				ObjectBlockCompositionSelection = Processor::Bit2,
				BackgroundCodeAreaSelection = Processor::Bit3,
				BackgroundCharacterDataSelection = Processor::Bit4,
				WindowEnable = Processor::Bit5,
				WindowCodeAreaSelection = Processor::Bit6,
				LcdEnable = Processor::Bit7
			};

			enum LcdStatusMode {
				HBlank = 0b00,
				VBlank = 0b01,
				SearchingOamRam = 0b10,
				TransferringDataToLcd = 0b11
			};

			IoRegisters(Bus& bus);

			Signal<int> DisplayStatusModeUpdated;

			void reset();

			void transferDma();

			void triggerInterrupt(int cause) {
				poke(IF, peek(IF) | cause);
			}

			void checkTimers(int cycles);

			int timerClockTicks();

			int timerClock();
			bool timerEnabled();
			bool timerDisabled();

			void incrementDIV(int cycles);
			void incrementTIMA();

			void incrementLY();
			void resetLY();

			void updateLcdStatusMode(int mode);

			void disableBootRom() { m_disableBootRom = true; }
			void enableBootRom() { m_disableBootRom = false; }

			bool bootRomDisabled() const { return m_disableBootRom; }
			bool bootRomEnabled() const { return !bootRomDisabled(); }

			void pressRight() { m_p14 = m_p10 = false; triggerKeypadInterrupt(); }
			void releaseRight() { m_p14 = m_p10 = true; }

			void pressLeft() { m_p14 = m_p11 = false, triggerKeypadInterrupt(); }
			void releaseLeft() { m_p14 = m_p11 = true; }

			void pressUp() { m_p14 = m_p12 = false, triggerKeypadInterrupt(); }
			void releaseUp() { m_p14 = m_p12 = true; }

			void pressDown() { m_p14 = m_p13 = false, triggerKeypadInterrupt(); }
			void releaseDown() { m_p14 = m_p13 = true; }

			void pressA() { m_p15 = m_p10 = false, triggerKeypadInterrupt(); }
			void releaseA() { m_p15 = m_p10 = true; }

			void pressB() { m_p15 = m_p11 = false, triggerKeypadInterrupt(); }
			void releaseB() { m_p15 = m_p11 = true; }

			void pressSelect() { m_p15 = m_p12 = false, triggerKeypadInterrupt(); }
			void releaseSelect() { m_p15 = m_p12 = true; }

			void pressStart() { m_p15 = m_p13 = false, triggerKeypadInterrupt(); }
			void releaseStart() { m_p15 = m_p13 = true; }

		private:
			Bus& m_bus;

			bool m_disableBootRom = false;

			register16_t m_divCounter = { 0xab, 0xcc };
			int m_timerCounter = 0;
			int m_timerRate = 0;

			register16_t m_dmaAddress = { 0, 0 };
			bool m_dmaTransferActive = false;

			bool m_scanP15 = false;
			bool m_scanP14 = false;

			bool m_p15 = true;	// misc keys
			bool m_p14 = true;	// direction keys
			bool m_p13 = true;	// down/start
			bool m_p12 = true;	// up/select
			bool m_p11 = true;	// left/b
			bool m_p10 = true;	// right/a

			void checkTimer(int cycles);

			void mask(uint16_t address, uint8_t masking) {
				poke(address, peek(address) | ~masking);
			}

			void triggerKeypadInterrupt() {
				triggerInterrupt(Interrupts::KeypadPressed);
			}

			void Bus_WrittenByte(uint16_t address);
			void Bus_ReadingByte(uint16_t address);
		};
	}
}