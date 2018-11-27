#pragma once

#include <string>

#include <Register.h>

class Configuration final {
public:
	Configuration() noexcept = default;

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

	const std::string& getRomDirectory() const {
		return m_romDirectory;
	}

	EightBit::register16_t getStartAddress() const {
		return 0x100;
	}

private:
	bool m_debugMode = false;
	bool m_profileMode = false;
	std::string m_romDirectory = "roms";
};
