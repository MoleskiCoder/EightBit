#pragma once

#include <cstdint>
#include <vector>

#include <Rom.h>
#include <Ram.h>
#include <Bus.h>

#include "LR35902.h"
#include "IoRegisters.h"

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

			Bus();

			LR35902& CPU() { return m_cpu; }
			Ram& VRAM() { return m_videoRam; }
			Ram& OAMRAM() { return m_oamRam; }
			IoRegisters& IO() { return m_ioPorts; }

			void reset();

			void disableGameRom() { m_disableGameRom = true; }
			void enableGameRom() { m_disableGameRom = false; }

			bool gameRomDisabled() const { return m_disableGameRom; }
			bool gameRomEnabled() const { return !gameRomDisabled(); }

			void loadBootRom(const std::string& path);
			void loadGameRom(const std::string& path);

		protected:
			virtual uint8_t& reference(uint16_t address, bool& rom);

		private:
			LR35902 m_cpu;

			Rom m_bootRom;						// 0x0000 - 0x00ff
			std::vector<Rom> m_gameRomBanks;	// 0x0000 - 0x3fff, 0x4000 - 0x7fff (switchable)
			Ram m_videoRam;						// 0x8000 - 0x9fff
			std::vector<Ram> m_ramBanks;		// 0xa000 - 0xbfff (switchable)
			Ram m_lowInternalRam;				// 0xc000 - 0xdfff (mirrored at 0xe000)
			Ram m_oamRam;						// 0xfe00 - 0xfe9f
			IoRegisters m_ioPorts;				// 0xff00 - 0xff7f
			Ram m_highInternalRam;				// 0xff80 - 0xffff

			bool m_disableGameRom;

			bool m_rom;
			bool m_banked;
			bool m_ram;
			bool m_battery;

			bool m_higherRomBank;
			bool m_ramBankSwitching;

			int m_romBank;
			int m_ramBank;

			void validateCartridgeType();

			void Bus_WrittenByte(uint16_t address);
		};
	}
}