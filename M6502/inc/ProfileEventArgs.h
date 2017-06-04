#pragma once

#include <string>

class ProfileEventArgs
{
private:
	std::string m_output;

public:
	ProfileEventArgs(std::string output)
	: m_output(output) {}

	const std::string& getOutput() const { return m_output;	 }
};