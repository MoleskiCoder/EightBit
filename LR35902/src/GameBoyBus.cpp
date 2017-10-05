#include "stdafx.h"
#include "GameBoyBus.h"

EightBit::GameBoy::Bus::Bus()
: m_bootRom(0x100),
  m_gameRomBanks(1),
  m_videoRam(0x2000),
  m_ramBanks(0),
  m_lowInternalRam(0x2000),
  m_oamRam(0xa0),
  m_ioPorts(*this),
  m_highInternalRam(0x80),
  m_disableGameRom(false),
  m_rom(false),
  m_banked(false),
  m_ram(false),
  m_battery(false),
  m_higherRomBank(true),
  m_ramBankSwitching(false),
  m_romBank(1),
  m_ramBank(0) {
	WrittenByte.connect(std::bind(&GameBoy::Bus::Bus_WrittenByte, this, std::placeholders::_1));
}

void EightBit::GameBoy::Bus::reset() {
	m_ioPorts.reset();
}

void EightBit::GameBoy::Bus::loadBootRom(const std::string& path) {
	m_bootRom.load(path);
}

void EightBit::GameBoy::Bus::loadGameRom(const std::string& path) {
	const auto bankSize = 0x4000;
	m_gameRomBanks.resize(1);
	const auto size = m_gameRomBanks[0].load(path, 0, 0, bankSize);
	const auto banks = size / bankSize;
	m_gameRomBanks.resize(banks);
	for (int bank = 1; bank < banks; ++bank)
		m_gameRomBanks[bank].load(path, 0, bankSize * bank, bankSize);
	validateCartridgeType();
}

void EightBit::GameBoy::Bus::Bus_WrittenByte(const uint16_t address) {

	const auto value = DATA();

	switch (address & 0xe000) {
	case 0x0000:
		// Register 0: RAMCS gate data
		if (m_ram) {
			assert(false);
		}
		break;
	case 0x2000:
		// Register 1: ROM bank code
		if (m_banked && m_higherRomBank) {
			assert((address >= 0x2000) && (address < 0x4000));
			assert((value > 0) && (value < 0x20));
			m_romBank = value & Processor::Mask5;
		}
		break;
	case 0x4000:
		// Register 2: ROM bank selection
		if (m_banked) {
			assert(false);
		}
	case 0x6000:
		// Register 3: ROM/RAM change
		if (m_banked) {
			switch (value & Processor::Mask1) {
			case 0:
				m_higherRomBank = true;
				m_ramBankSwitching = false;
				break;
			case 1:
				m_higherRomBank = false;
				m_ramBankSwitching = true;
				break;
			default:
				UNREACHABLE;
			}
		}
		break;
	}
}

void EightBit::GameBoy::Bus::validateCartridgeType() {

	m_rom = m_banked = m_ram = m_battery = false;

	// ROM type
	switch (m_gameRomBanks[0].peek(0x147)) {
	case ROM:
		m_rom = true;
		break;
	case ROM_MBC1:
		m_rom = m_banked = true;
		break;
	case ROM_MBC1_RAM:
		m_rom = m_banked = m_ram = true;
		break;
	case ROM_MBC1_RAM_BATTERY:
		m_rom = m_banked = m_ram = m_battery = true;
		break;
	default:
		throw std::domain_error("Unhandled cartridge ROM type");
	}

	// ROM size
	{
		size_t gameRomBanks = 0;
		int romSizeSpecification = m_gameRomBanks[0].peek(0x148);
		switch (romSizeSpecification) {
		case 0x52:
			gameRomBanks = 72;
			break;
		case 0x53:
			gameRomBanks = 80;
			break;
		case 0x54:
			gameRomBanks = 96;
			break;
		default:
			if (romSizeSpecification > 6)
				throw std::domain_error("Invalid ROM size specification");
			gameRomBanks = 1 << (romSizeSpecification + 1);
			if (gameRomBanks != m_gameRomBanks.size())
				throw std::domain_error("ROM size specification mismatch");
		}

		// RAM size
		{
			auto ramSizeSpecification = m_gameRomBanks[0].peek(0x149);
			switch (ramSizeSpecification) {
			case 0:
				break;
			case 1:
				m_ramBanks.resize(1);
				m_ramBanks[0] = Ram(2 * 1024);
				break;
			case 2:
				m_ramBanks.resize(1);
				m_ramBanks[0] = Ram(8 * 1024);
				break;
			case 3:
				m_ramBanks.resize(4);
				for (int i = 0; i < 4; ++i)
					m_ramBanks[i] = Ram(8 * 1024);
				break;
			case 4:
				m_ramBanks.resize(16);
				for (int i = 0; i < 16; ++i)
					m_ramBanks[i] = Ram(8 * 1024);
				break;
			default:
				throw std::domain_error("Invalid RAM size specification");
			}
		}
	}
}

uint8_t& EightBit::GameBoy::Bus::reference(uint16_t address, bool& rom) {

	rom = true;
	if ((address < 0x100) && m_ioPorts.bootRomEnabled())
		return m_bootRom.reference(address);
	if ((address < 0x4000) && gameRomEnabled())
		return m_gameRomBanks[0].reference(address);
	if ((address < 0x8000) && gameRomEnabled())
		return m_gameRomBanks[m_romBank].reference(address - 0x4000);

	rom = false;
	if (address < 0xa000)
		return VRAM().reference(address - 0x8000);
	if (address < 0xc000)
		return m_ramBanks.size() == 0 ? rom = true, placeDATA(0xff) : m_ramBanks[m_ramBank].reference(address - 0xa000);
	if (address < 0xe000)
		return m_lowInternalRam.reference(address - 0xc000);
	if (address < 0xfe00)
		return m_lowInternalRam.reference(address - 0xe000);	// Low internal RAM mirror
	if (address < 0xfea0)
		return OAMRAM().reference(address - 0xfe00);
	if (address < IoRegisters::BASE)
		return rom = true, placeDATA(0xff);
	if (address < 0xff80)
		return m_ioPorts.reference(address - IoRegisters::BASE);
	return m_highInternalRam.reference(address - 0xff80);
}
