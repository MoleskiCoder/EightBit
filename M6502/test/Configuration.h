#pragma once

#include <cstdint>
#include <string>
#include <Register.h>

class Configuration {
public:
	enum StopCondition {
		Halt,
		Loop
	};

	enum LoadMethod {
		Ram,
		Rom
	};

	Configuration() = default;

	bool isDebugMode() const { return m_debugMode; }
	void setDebugMode(bool value) { m_debugMode = value; }

	bool isProfileMode() const { return m_profileMode; }
	void setProfileMode(bool value) { m_profileMode = value; }

	EightBit::register16_t getLoadAddress() const { return m_loadAddress; }
	EightBit::register16_t getStartAddress() const { return m_startAddress; }

	bool allowInput() const { return m_allowInput; }
	EightBit::register16_t getInputAddress() const { return m_inputAddress; }

	bool allowOutput() const { return m_allowOutput; }
	EightBit::register16_t getOutputAddress() const { return m_outputAddress; }

	uint64_t getPollInterval() const { return m_pollInterval; }

	StopCondition getStopCondition() const { return m_stopCondition; }

	std::string getRomDirectory() const { return m_romDirectory; }
	std::string getProgram() const { return m_program; }

	LoadMethod getLoadMethod() const { return m_loadMethod; }

private:
	bool m_debugMode = false;
	bool m_profileMode = false;
	EightBit::register16_t m_loadAddress = 0x400;
	EightBit::register16_t m_startAddress = 0x400;
	bool m_allowInput = false;
	EightBit::register16_t m_inputAddress = 0xbff0;
	bool m_allowOutput = false;
	EightBit::register16_t m_outputAddress = 0xbff0;
	uint64_t m_pollInterval = 2000000 / 50;
	StopCondition m_stopCondition = StopCondition::Loop;
	std::string m_romDirectory = "roms";
	std::string m_program = "6502_functional_test.bin";
	LoadMethod m_loadMethod = LoadMethod::Ram;
};
