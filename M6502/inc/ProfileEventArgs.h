#pragma once

#include <string>

namespace EightBit {
	class ProfileEventArgs {
	private:
		std::string m_output;

	public:
		ProfileEventArgs(std::string output)
			: m_output(output) {}

		const std::string& getOutput() const { return m_output; }
	};
}