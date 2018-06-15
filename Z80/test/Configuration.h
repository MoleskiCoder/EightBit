#pragma once

#include <string>

#include <Register.h>

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
		//EightBit::register16_t returned;
		//returned.word = 0x100;
		return 0x100;
	}

private:
	bool m_debugMode;
	bool m_profileMode;

	std::string m_romDirectory;
};
