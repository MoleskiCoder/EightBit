#pragma once

#include <string>

class DisassemblyEventArgs
{
private:
	std::string m_output;

public:
	DisassemblyEventArgs(std::string output)
	: m_output(output) {}

	const std::string& getOutput() const { return m_output; }
};