#pragma once

#include <array>
#include <tuple>

#include <Rom.h>
#include <Ram.h>
#include <Bus.h>
#include <Processor.h>
#include <Signal.h>

#include "Audio.h"

namespace EightBit {
	namespace GameBoy {
		class Bus : public EightBit::Bus {
		public:

			enum CartridgeType {
				ROM = 0,
				ROM_MBC1 = 1,
				ROM_MBC1_RAM = 2,
				ROM_MBC1_RAM_BATTERY = 3,
			};

			enum {
				CyclesPerSecond = 4 * 1024 * 1024,
				FramesPerSecond = 60,
				CyclesPerFrame = CyclesPerSecond / FramesPerSecond,
				TotalLineCount = 154,
				CyclesPerLine = CyclesPerFrame / TotalLineCount,
				RomPageSize = 0x4000
			};

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
				KeypadPressed = Processor::Bit3				// Hi-Lo transition of P10-P13
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

			Bus();

			Audio& audio() { return m_audio; }

			void reset();

			void triggerInterrupt(int cause) {
				pokeRegister(IF, peekRegister(IF) | cause);
			}

			void writeRegister(int offset, uint8_t content) {
				write(BASE + offset, content);
			}

			void pokeRegister(int offset, uint8_t content) {
				poke(BASE + offset, content);
			}

			uint8_t readRegister(int offset) {
				return read(BASE + offset);
			}

			uint8_t peekRegister(int offset) {
				return peek(BASE + offset);
			}

			void checkTimers(int cycles);

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
				return peekRegister(TAC) & Processor::Mask2;
			}

			bool timerEnabled() {
				return !timerDisabled();
			}

			bool timerDisabled() {
				return (peekRegister(TAC) & Processor::Bit2) == 0;
			}

			void incrementDIV(int cycles) {
				m_divCounter.word += cycles;
				pokeRegister(DIV, m_divCounter.high);
			}

			void incrementTIMA() {
				uint16_t updated = peekRegister(TIMA) + 1;
				if (updated & Processor::Bit8) {
					triggerInterrupt(TimerOverflow);
					updated = peekRegister(TMA);
				}
				pokeRegister(TIMA, updated & Processor::Mask8);
			}

			void incrementLY() {
				pokeRegister(LY, (peekRegister(LY) + 1) % TotalLineCount);
			}

			void resetLY() {
				pokeRegister(LY, 0);
			}

			void transferDma() {
				if (m_dmaTransferActive) {
					m_oamRam.poke(m_dmaAddress.low, peek(m_dmaAddress.word));
					m_dmaTransferActive = ++m_dmaAddress.low < 0xa0;
				}
			}

			void updateLcdStatusMode(int mode) {
				const auto current = m_ioPorts.peek(STAT) & ~Processor::Mask2;
				m_ioPorts.poke(STAT, current | mode);
			}

			void disableBootRom() { m_disableBootRom = true; }
			void enableBootRom() { m_disableBootRom = false; }

			void disableGameRom() { m_disableGameRom = true; }
			void enableGameRom() { m_disableGameRom = false; }

			bool bootRomDisabled() const { return m_disableBootRom; }
			bool bootRomEnabled() const { return !bootRomDisabled(); }

			bool gameRomDisabled() const { return m_disableGameRom; }
			bool gameRomEnabled() const { return !gameRomDisabled(); }

			void loadBootRom(const std::string& path);
			void loadGameRom(const std::string& path);

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

		protected:
			virtual uint8_t& reference(uint16_t address, bool& rom) {

				rom = true;
				if ((address < 0x100) && bootRomEnabled())
					return m_bootRom.reference(address);
				if ((address < 0x4000) && gameRomEnabled())
					return m_gameRomBanks[0].reference(address);
				if ((address < 0x8000) && gameRomEnabled())
					return m_gameRomBanks[m_romBank].reference(address - 0x4000);

				rom = false;
				if (address < 0xa000)
					return m_videoRam.reference(address - 0x8000);
				if (address < 0xc000)
					return m_ramBanks.size() == 0 ? rom = true, placeDATA(0xff) : m_ramBanks[m_ramBank].reference(address - 0xa000);
				if (address < 0xe000)
					return m_lowInternalRam.reference(address - 0xc000);
				if (address < 0xfe00)
					return m_lowInternalRam.reference(address - 0xe000);	// Low internal RAM mirror
				if (address < 0xfea0)
					return m_oamRam.reference(address - 0xfe00);
				if (address < 0xff00)
					return rom = true, placeDATA(0xff);
				if (address < 0xff80)
					return m_ioPorts.reference(address - 0xff00);
				return m_highInternalRam.reference(address - 0xff80);
			}

		private:
			Rom m_bootRom;						// 0x0000 - 0x00ff
			std::vector<Rom> m_gameRomBanks;	// 0x0000 - 0x3fff, 0x4000 - 0x7fff (switchable)
			Ram m_videoRam;						// 0x8000 - 0x9fff
			std::vector<Ram> m_ramBanks;		// 0xa000 - 0xbfff (switchable)
			Ram m_lowInternalRam;				// 0xc000 - 0xdfff (mirrored at 0xe000)
			Ram m_oamRam;						// 0xfe00 - 0xfe9f
			Ram m_ioPorts;						// 0xff00 - 0xff7f
			Ram m_highInternalRam;				// 0xff80 - 0xffff

			bool m_disableBootRom;
			bool m_disableGameRom;

			bool m_rom;
			bool m_banked;
			bool m_ram;
			bool m_battery;

			bool m_higherRomBank;
			bool m_ramBankSwitching;

			int m_romBank;
			int m_ramBank;

			register16_t m_divCounter;
			int m_timerCounter;
			int m_timerRate;

			register16_t m_dmaAddress;
			bool m_dmaTransferActive;

			bool m_scanP15;
			bool m_scanP14;

			bool m_p15;	// misc keys
			bool m_p14;	// direction keys
			bool m_p13;	// down/start
			bool m_p12;	// up/select
			bool m_p11;	// left/b
			bool m_p10;	// right/a

			Audio m_audio;

			void checkTimer(int cycles);

			void validateCartridgeType();

			void mask(uint16_t address, uint8_t masking) {
				poke(address, peek(address) | ~masking);
			}

			void mask(uint8_t masking) {
				mask(ADDRESS().word, masking);
			}

			void triggerKeypadInterrupt() {
				//triggerInterrupt(Interrupts::KeypadPressed);
			}

			void Bus_WrittenByte(uint16_t address);
			void Bus_ReadingByte(uint16_t address);
		};
	}
}