#pragma once

#include <cstdint>
#include <string>
#include <Register.h>

class Configuration {
public:
	Configuration() = default;

	bool isDebugMode() const { return m_debugMode; }
	void setDebugMode(bool value) { m_debugMode = value; }

	EightBit::register16_t getLoadAddress() const { return m_loadAddress; }
	EightBit::register16_t getStartAddress() const { return m_startAddress; }

	std::string getRomDirectory() const { return m_romDirectory; }
	std::string getProgram() const { return m_program; }

private:
	bool m_debugMode = false;
	EightBit::register16_t m_loadAddress = 0x400;
	EightBit::register16_t m_startAddress = 0x400;
	std::string m_romDirectory = "roms";
	std::string m_program = "6502_functional_test.bin";
};
