#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <Ram.h>
#include <Rom.h>
#include <Bus.h>
#include <Register.h>
#include <UnusedMemory.h>

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

			Bus() noexcept;

			virtual void raisePOWER() override;
			virtual void lowerPOWER() override;

			[[nodiscard]] auto& CPU() noexcept { return m_cpu; }
			[[nodiscard]] auto& VRAM() noexcept { return m_videoRam; }
			[[nodiscard]] auto& OAMRAM() noexcept { return m_oamRam; }
			[[nodiscard]] auto& IO() noexcept { return m_ioPorts; }

			void reset();

			void disableGameRom() noexcept { m_disableGameRom = true; }
			void enableGameRom() noexcept { m_disableGameRom = false; }

			[[nodiscard]] bool gameRomDisabled() const noexcept { return m_disableGameRom; }
			[[nodiscard]] bool gameRomEnabled() const noexcept { return !gameRomDisabled(); }

			void loadBootRom(std::string path);
			void loadGameRom(std::string path);

			void runRasterLines();
			void runVerticalBlankLines();

		protected:
			virtual MemoryMapping mapping(uint16_t address) noexcept override;

		private:
			LR35902 m_cpu;

			Rom m_bootRom = 0x100;							// 0x0000 - 0x00ff
			std::vector<Rom> m_gameRomBanks;				// 0x0000 - 0x3fff, 0x4000 - 0x7fff (switchable)
			Ram m_videoRam = 0x2000;						// 0x8000 - 0x9fff
			std::vector<Ram> m_ramBanks;					// 0xa000 - 0xbfff (switchable)
			UnusedMemory m_unmapped2000 = { 0x2000, 0xff };	// 0xa000 - 0xbfff
			Ram m_lowInternalRam = 0x2000;					// 0xc000 - 0xdfff (mirrored at 0xe000)
			Ram m_oamRam = 0xa0;							// 0xfe00 - 0xfe9f
			UnusedMemory m_unmapped60 = { 0x60, 0xff };		// 0xfea0 - 0xfeff
			IoRegisters m_ioPorts;							// 0xff00 - 0xff7f
			Ram m_highInternalRam = 0x80;					// 0xff80 - 0xffff

			bool m_enabledLCD = false;

			bool m_disableGameRom = false;

			bool m_rom = false;
			bool m_banked = false;
			bool m_ram = false;
			bool m_battery = false;

			bool m_higherRomBank = true;
			bool m_ramBankSwitching = false;

			int m_romBank = 1;
			int m_ramBank = 0;

			int m_allowed = 0;

			void validateCartridgeType();

			void Bus_WrittenByte(EightBit::EventArgs) noexcept;

			void runRasterLines(int lines);
			void runVerticalBlankLines(int lines);
			void runRasterLine(int suggested);
		};
	}
}