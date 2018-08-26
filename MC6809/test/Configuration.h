#pragma once

#include <cstdint>
#include <string>
#include <Register.h>

class Configuration {
public:
	Configuration() = default;

	bool isDebugMode() const { return m_debugMode; }
	void setDebugMode(bool value) { m_debugMode = value; }

	std::string getRomDirectory() const { return m_romDirectory; }

private:
	bool m_debugMode = false;
	std::string m_romDirectory = "roms\\coco2h";
};
