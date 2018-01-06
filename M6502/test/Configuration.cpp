#include "stdafx.h"
#include "Configuration.h"

Configuration::Configuration()
: m_debugMode(false),
  m_profileMode(false),
  m_loadAddress(0x400),
  m_startAddress(0x400),
  m_allowInput(false),
  m_inputAddress(0xbff0),
  m_allowOutput(false),
  m_outputAddress(0xbff0),
  m_pollInterval(2000000 / 50), // 2Mhz, 50 times a second;
  m_stopCondition(StopCondition::Loop),
  m_romDirectory("roms"),
  m_program("6502_functional_test.bin"),
  m_loadMethod(LoadMethod::Ram) {
}
