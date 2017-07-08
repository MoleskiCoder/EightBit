#pragma once

#include <string>

#include "Memory.h"

class Configuration {
public:
	Configuration();

	bool isDebugMode() const {
		return m_debugMode;
	}

	void setDebugMode(bool value) {
		m_debugMode = value;
	}

	bool isProfileMode() const {
		return m_profileMode;
	}

	void setProfileMode(bool value) {
		m_profileMode = value;
	}

	std::string getRomDirectory() const {
		return m_romDirectory;
	}

	EightBit::register16_t getStartAddress() const {
		EightBit::register16_t returned;
		returned.word = 0x400;
		return returned;
	}

	bool allowInput() const { return false; }
	uint16_t getInputAddress() const { return 0xbff0; }

	bool allowOutput() const { return false; }
	uint16_t getOutputAddress() const { return 0xbff0; }

	uint64_t getPollInterval() const {
		return 2000000 / 50; // 2Mhz, 50 times a second;
	}

private:
	bool m_debugMode;
	bool m_profileMode;

	std::string m_romDirectory;
};
