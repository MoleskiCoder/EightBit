#pragma once

#include <cstdint>
#include <string>

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

	Configuration();

	bool isDebugMode() const { return m_debugMode; }
	void setDebugMode(bool value) { m_debugMode = value; }

	bool isProfileMode() const { return m_profileMode; }
	void setProfileMode(bool value) { m_profileMode = value; }

	uint16_t getLoadAddress() const { return m_loadAddress; }
	uint16_t getStartAddress() const { return m_startAddress; }

	bool allowInput() const { return m_allowInput; }
	uint16_t getInputAddress() const { return m_inputAddress; }

	bool allowOutput() const { return m_allowOutput; }
	uint16_t getOutputAddress() const { return m_outputAddress; }

	uint64_t getPollInterval() const { return m_pollInterval; }

	StopCondition getStopCondition() const { return m_stopCondition; }

	std::string getRomDirectory() const { return m_romDirectory; }
	std::string getProgram() const { return m_program; }

	LoadMethod getLoadMethod() const { return m_loadMethod; }

private:
	bool m_debugMode;
	bool m_profileMode;
	uint16_t m_loadAddress;
	uint16_t m_startAddress;
	bool m_allowInput;
	uint16_t m_inputAddress;
	bool m_allowOutput;
	uint16_t m_outputAddress;
	uint64_t m_pollInterval;
	StopCondition m_stopCondition;
	std::string m_romDirectory;
	std::string m_program;
	LoadMethod m_loadMethod;
};
