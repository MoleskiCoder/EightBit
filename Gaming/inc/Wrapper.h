#pragma once

#include <string>
#include <stdexcept>
#include <sstream>

namespace Gaming {
	class Wrapper final {
	public:
		Wrapper(bool verbose = false);
		~Wrapper();

		static void throwException(const std::string& failure) {
			std::ostringstream output;
			output << "SDL: " << failure << ": " << ::SDL_GetError();
			throw std::runtime_error(output.str());
		}

		static void maybeThrowException(bool success, const std::string& failure) {
			if (!success)
				throwException(failure);
		}

		static void maybeThrowException(void* handle, const std::string& failure) {
			maybeThrowException(handle != nullptr, failure);
		}
	};
}