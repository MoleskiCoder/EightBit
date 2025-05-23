#pragma once

#include <cstdint>
#include <string>

class Configuration final {
public:
	static constexpr uint64_t CyclesPerSecond = 2 * 1024 * 1024;
	static constexpr uint64_t FrameCycleInterval = CyclesPerSecond / 60;
	static constexpr uint64_t TerminationCycles = CyclesPerSecond * 10 * 10;

	Configuration() = default;

	bool isDebugMode() const { return m_debugMode; }
	void setDebugMode(bool value) { m_debugMode = value; }

	bool isProfileMode() const { return m_profileMode; }
	void setProfileMode(bool value) { m_profileMode = value; }

	bool terminatesEarly() const { return m_terminatesEarly; }
	void setTerminatesEarly(bool value) { m_terminatesEarly = value; }

	std::string getRomDirectory() const { return m_romDirectory; }

private:
	bool m_debugMode = false;
	bool m_profileMode = false;
	bool m_terminatesEarly = false;
	std::string m_romDirectory = "roms\\searle";
};
