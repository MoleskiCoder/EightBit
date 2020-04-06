#include "stdafx.h"
#include "IoRegisters.h"
#include "GameBoyBus.h"

EightBit::GameBoy::IoRegisters::IoRegisters(Bus& bus)
: Ram(0x80),
  m_bus(bus) {
	m_bus.ReadingByte.connect(std::bind(&IoRegisters::Bus_ReadingByte, this, std::placeholders::_1));
	m_bus.WrittenByte.connect(std::bind(&IoRegisters::Bus_WrittenByte, this, std::placeholders::_1));
}

void EightBit::GameBoy::IoRegisters::reset() {
	poke(NR52, 0xf1);
	poke(LCDC, DisplayBackground | BackgroundCharacterDataSelection | LcdEnable);
	m_divCounter = 0xabcc;
	m_timerCounter = 0;
}

void EightBit::GameBoy::IoRegisters::transferDma() {
	if (m_dmaTransferActive) {
		m_bus.OAMRAM().poke(m_dmaAddress.low, m_bus.peek(m_dmaAddress));
		m_dmaTransferActive = ++m_dmaAddress.low < 0xa0;
	}
}

void EightBit::GameBoy::IoRegisters::Bus_ReadingByte(EightBit::EventArgs) {
	const auto address = m_bus.ADDRESS().word;
	const auto io = (address >= BASE) && (address < 0xff80);
	if (io) {
		auto port = address - BASE;
		switch (port) {

		// Port/Mode Registers
		case P1: {
				auto directionKeys = m_scanP14 && !m_p14;
				auto miscKeys = m_scanP15 && !m_p15;
				auto live = directionKeys || miscKeys;
				auto rightOrA = (live && !m_p10) ? 0 : Chip::Bit0;
				auto leftOrB = (live && !m_p11) ? 0 : Chip::Bit1;
				auto upOrSelect = (live && !m_p12) ? 0 : Chip::Bit2;
				auto downOrStart = (live && !m_p13) ? 0 : Chip::Bit3;
				auto lowNibble = rightOrA | leftOrB | upOrSelect | downOrStart;
				constexpr auto highNibble = Chip::promoteNibble(Chip::Mask4);
				auto value = lowNibble | highNibble;
				poke(port, lowNibble | highNibble);
			}
			break;
		case SB:
			break;
		case SC:
			mask(port, Chip::Bit7 | Chip::Bit0);
			break;

		// Timer control
		case DIV:
		case TIMA:
		case TMA:
			break;
		case TAC:
			mask(port, Chip::Mask3);
			break;

		// Interrupt Flags
		case IF:
			mask(port, Chip::Mask5);
			break;

		// LCD Display Registers
		case LCDC:
			break;
		case STAT:
			mask(port, Chip::Mask7);
			break;
		case SCY:
		case SCX:
		case LY:
		case LYC:
		case DMA:
		case BGP:
		case OBP0:
		case OBP1:
		case WY:
		case WX:
			break;

		default:
			mask(port, 0);
			break;
		}
	}
}

void EightBit::GameBoy::IoRegisters::Bus_WrittenByte(EightBit::EventArgs) {

	const auto address = m_bus.ADDRESS().word;
	const auto value = m_bus.DATA();
	const auto port = address - BASE;

	switch (port) {

	case P1:
		m_scanP14 = (value & Chip::Bit4) == 0;
		m_scanP15 = (value & Chip::Bit5) == 0;
		break;

	case SB:		// R/W
	case SC:		// R/W
		break;

	case DIV:	// R/W
		poke(port, 0);
		m_timerCounter = m_divCounter.word = 0;
		break;
	case TIMA:	// R/W
	case TMA:	// R/W
		break;
	case TAC:	// R/W
		m_timerRate = timerClockTicks();
		break;

	case IF:		// R/W
		break;

	case LCDC:
	case STAT:
	case SCY:
	case SCX:
		break;
	case DMA:
		m_dmaAddress = register16_t(0, value);
		m_dmaTransferActive = true;
		break;
	case LY:		// R/O
		poke(port, 0);
		break;
	case BGP:
	case OBP0:
	case OBP1:
	case WY:
	case WX:
		break;

	case BOOT_DISABLE:
		m_disableBootRom = !!value;
		break;
	}
}

void EightBit::GameBoy::IoRegisters::checkTimers(int cycles) {
	incrementDIV(cycles);
	checkTimer(cycles);
}

void EightBit::GameBoy::IoRegisters::checkTimer(int cycles) {
	if (timerEnabled()) {
		m_timerCounter -= cycles;
		if (m_timerCounter <= 0) {
			m_timerCounter += m_timerRate;
			incrementTIMA();
		}
	}
}

int EightBit::GameBoy::IoRegisters::timerClockTicks() {
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
		UNREACHABLE;
	}
	throw std::domain_error("Invalid timer clock specification");
}

int EightBit::GameBoy::IoRegisters::timerClock() {
	return peek(TAC) & Chip::Mask2;
}

bool EightBit::GameBoy::IoRegisters::timerEnabled() {
	return !timerDisabled();
}

bool EightBit::GameBoy::IoRegisters::timerDisabled() {
	return (peek(TAC) & Chip::Bit2) == 0;
}

void EightBit::GameBoy::IoRegisters::incrementDIV(int cycles) {
	m_divCounter += cycles;
	poke(DIV, m_divCounter.high);
}

void EightBit::GameBoy::IoRegisters::incrementTIMA() {
	uint16_t updated = peek(TIMA) + 1;
	if (updated & Chip::Bit8) {
		triggerInterrupt(TimerOverflow);
		updated = peek(TMA);
	}
	poke(TIMA, updated & Chip::Mask8);
}

void EightBit::GameBoy::IoRegisters::incrementLY() {
	poke(LY, (peek(LY) + 1) % GameBoy::Bus::TotalLineCount);
}

void EightBit::GameBoy::IoRegisters::resetLY() {
	poke(LY, 0);
}

void EightBit::GameBoy::IoRegisters::updateLcdStatusMode(int mode) {
	const auto current = peek(STAT) & ~Chip::Mask2;
	poke(STAT, current | mode);
	DisplayStatusModeUpdated.fire(mode);
}
